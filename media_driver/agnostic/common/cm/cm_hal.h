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
//! \file      cm_hal.h 
//! \brief     Main Entry point for CM HAL component 
//!
#ifndef __CM_HAL_H__
#define __CM_HAL_H__

#include "renderhal.h"
#include "cm_common.h"
#include "cm_debug.h"
#include "cm_csync.h"
#include "mhw_vebox.h"
#include "cm_hal_generic.h"
#include "media_perf_profiler.h"
#include <string>
#include <map>

#define DiscardLow8Bits(x)  (uint16_t)(0xff00 & x)
#define FloatToS3_12(x)  (uint16_t)((short)(x * 4096))
#define FloatToS3_4(x)   (DiscardLow8Bits(FloatToS3_12(x)))

#define CM_32K                      (32*1024)

#define HAL_CM_KERNEL_CACHE_MISS_THRESHOLD    4
#define HAL_CM_KERNEL_CACHE_HIT_TO_MISS_RATIO 100

#ifdef _WIN64
#define IGC_DLL_NAME   "igc64.dll"
#else
#define IGC_DLL_NAME   "igc32.dll"
#endif  // _WIN64

#define SURFACE_FLAG_ASSUME_NOT_IN_USE 1
#define CM_THREADSPACE_MAX_COLOR_COUNT 16

static const uint32_t INVALID_STREAM_INDEX = 0xFFFFFFFF;

//*-----------------------------------------------------------------------------
//| Macro unsets bitPos bit in value
//*-----------------------------------------------------------------------------
#ifndef CM_HAL_UNSETBIT
#define CM_HAL_UNSETBIT(value, bitPos)                                         \
{                                                                              \
    value = (value & ~(1 << bitPos));                                          \
}
#endif

#ifndef CM_HAL_UNSETBIT64
#define CM_HAL_UNSETBIT64(value, bitPos)                                       \
{                                                                              \
    value = (value & ~(1i64 << bitPos));                                       \
}
#endif

//*-----------------------------------------------------------------------------
//| Macro sets bitPos bit in value
//*-----------------------------------------------------------------------------
#ifndef CM_HAL_SETBIT
#define CM_HAL_SETBIT(value, bitPos)                                           \
{                                                                              \
    value = (value | (1 << bitPos));                                           \
}
#endif

#ifndef CM_HAL_SETBIT64
#define CM_HAL_SETBIT64(value, bitPos)                                         \
{                                                                              \
    value = (value | (1i64 << bitPos));                                        \
}
#endif

//*-----------------------------------------------------------------------------
//| Macro checks if bitPos bit in value is set
//*-----------------------------------------------------------------------------
#ifndef CM_HAL_CHECKBIT_IS_SET
#define CM_HAL_CHECKBIT_IS_SET(bitIsSet, value, bitPos)                        \
{                                                                              \
    bitIsSet = ((value) & (1 << bitPos))? true: false;                                      \
}
#endif

#ifndef CM_HAL_CHECKBIT_IS_SET64
#define CM_HAL_CHECKBIT_IS_SET64(bitIsSet, value, bitPos)                      \
{                                                                              \
    bitIsSet = ((value) & (1i64 << bitPos));                                   \
}
#endif

#define MAX_CUSTOMIZED_PERFTAG_INDEX    249
#define GPUINIT_PERFTAG_INDEX           250
#define GPUCOPY_READ_PERFTAG_INDEX      251
#define GPUCOPY_WRITE_PERFTAG_INDEX     252
#define GPUCOPY_G2G_PERFTAG_INDEX       253
#define GPUCOPY_C2C_PERFTAG_INDEX       254
#define VEBOX_TASK_PERFTAG_INDEX        255

#define MAX_COMBINE_NUM_IN_PERFTAG      16

#define MAX_ELEMENT_TYPE_COUNT 6

//Copied over from auxilarydevice to avoid inclusion of video acceleration APIs.
struct CMLOOKUP_ENTRY
{
    void *osSurfaceHandle;            // Surface defined in video acceleration APIs provided by the OS.
    UMD_RESOURCE umdSurfaceHandle;    // Surface defined in UMD.
    uint32_t surfaceAllocationIndex;  // Driver allocation index
};
typedef CMLOOKUP_ENTRY *PCMLOOKUP_ENTRY;

struct CMSURFACE_REG_TABLE
{
    uint32_t count;            // Number of entries
    CMLOOKUP_ENTRY *entries;  // Surface Lookup table
};

struct CM_HAL_MAX_VALUES
{
    uint32_t maxTasks;                 // [in] Max Tasks
    uint32_t maxKernelsPerTask;        // [in] Max kernels per task
    uint32_t maxKernelBinarySize;      // [in] Max kernel binary size
    uint32_t maxSpillSizePerHwThread;  // [in] Max spill size per thread
    uint32_t maxSamplerTableSize;      // [in] Max sampler table size
    uint32_t maxBufferTableSize;       // [in] Max buffer/bufferUP table Size
    uint32_t max2DSurfaceTableSize;    // [in] Max 2D surface table Size
    uint32_t max3DSurfaceTableSize;    // [in] Max 3D surface table Size
    uint32_t maxArgsPerKernel;         // [in] Max arguments per kernel
    uint32_t maxArgByteSizePerKernel;  // [in] Max argument size in byte per kernel
    uint32_t maxSurfacesPerKernel;     // [in] Max Surfaces Per Kernel
    uint32_t maxSamplersPerKernel;     // [in] Max Samplers per kernel
    uint32_t maxHwThreads;             // [in] Max HW threads
    uint32_t maxUserThreadsPerTask;    // [in] Max user threads per task
    uint32_t maxUserThreadsPerTaskNoThreadArg;  // [in] Max user threads per task with no thread arg
};
typedef CM_HAL_MAX_VALUES *PCM_HAL_MAX_VALUES;

//------------------------------------------------------------------------------------------------
//| HAL CM Max Values extention which has more entries which are not included in CM_HAL_MAX_VALUES
//-------------------------------------------------------------------------------------------------
struct CM_HAL_MAX_VALUES_EX
{
    uint32_t max2DUPSurfaceTableSize;       // [in] Max 2D UP surface table Size
    uint32_t maxSampler8x8TableSize;        // [in] Max sampler 8x8 table size
    uint32_t maxCURBESizePerKernel;         // [in] Max CURBE size per kernel
    uint32_t maxCURBESizePerTask;           // [in] Max CURBE size per task
    uint32_t maxIndirectDataSizePerKernel;  // [in] Max indirect data size per kernel
    uint32_t maxUserThreadsPerMediaWalker;  // [in] Max user threads per media walker
    uint32_t maxUserThreadsPerThreadGroup;  // [in] Max user threads per thread group
};
typedef CM_HAL_MAX_VALUES_EX *PCM_HAL_MAX_VALUES_EX;

struct CM_INDIRECT_SURFACE_INFO
{
    uint16_t kind;               // Surface kind, values in CM_ARG_KIND. For now, only support ARG_KIND_SURFACE_1D/ARG_KIND_SURFACE_2D/ARG_KIND_SURFACE_2D_UP
    uint16_t surfaceIndex;       // Surface handle used in driver
    uint16_t bindingTableIndex;  // Binding table index
    uint16_t numBTIPerSurf;      // Binding table index count for per surface
};
typedef CM_INDIRECT_SURFACE_INFO *PCM_INDIRECT_SURFACE_INFO;

//*-----------------------------------------------------------------------------
//| HAL CM Indirect Data Param
//*-----------------------------------------------------------------------------
struct CM_HAL_INDIRECT_DATA_PARAM
{
    uint16_t indirectDataSize;  // [in] Indirect Data Size
    uint16_t surfaceCount;
    uint8_t *indirectData;      // [in] Pointer to Indirect Data Block
    PCM_INDIRECT_SURFACE_INFO surfaceInfo;
};
typedef CM_HAL_INDIRECT_DATA_PARAM *PCM_HAL_INDIRECT_DATA_PARAM;

//------------------------
//| HAL CM Create Param
//------------------------
struct CM_HAL_CREATE_PARAM
{
    bool disableScratchSpace;           // Flag to disable Scratch Space
    uint32_t scratchSpaceSize;          // Size of Scratch Space per HW thread
    uint32_t maxTaskNumber;             // Max Task Number
    bool requestSliceShutdown;         // Flag to enable slice shutdown
    bool requestCustomGpuContext;      // Flag to use CUSTOM GPU Context
    uint32_t kernelBinarySizeinGSH;     // Size to be reserved in GSH for kernel binary
    bool dynamicStateHeap;             // Use Dynamic State Heap management
    bool disabledMidThreadPreemption;  // Flag to enable mid thread preemption for GPGPU
    bool enabledKernelDebug;           // Flag  to enable Kernel debug
    bool refactor;                     // Flag to enable the fast path
    bool disableVebox;                 // Flag to disable VEBOX API
};
typedef CM_HAL_CREATE_PARAM *PCM_HAL_CREATE_PARAM;

//------------------------------------------------------------------------------
//| CM Sampler Param
//------------------------------------------------------------------------------
enum CM_HAL_PIXEL_TYPE
{
    CM_HAL_PIXEL_UINT,
    CM_HAL_PIXEL_SINT,
    CM_HAL_PIXEL_OTHER
};

struct CM_HAL_SAMPLER_PARAM
{
    uint32_t magFilter;  // [in]  Mag Filter
    uint32_t minFilter;  // [in]  Min Filter
    uint32_t addressU;   // [in]  Address U
    uint32_t addressV;   // [in]  Address V
    uint32_t addressW;   // [in]  Address W
    uint32_t handle;   // [out] Handle

    CM_HAL_PIXEL_TYPE surfaceFormat;
    union
    {
        uint32_t borderColorRedU;
        int32_t borderColorRedS;
        float borderColorRedF;
    };

    union
    {
        uint32_t borderColorGreenU;
        int32_t borderColorGreenS;
        float borderColorGreenF;
    };

    union
    {
        uint32_t borderColorBlueU;
        int32_t borderColorBlueS;
        float borderColorBlueF;
    };

    union
    {
        uint32_t borderColorAlphaU;
        int32_t borderColorAlphaS;
        float borderColorAlphaF;
    };
};
typedef CM_HAL_SAMPLER_PARAM *PCM_HAL_SAMPLER_PARAM;

struct CM_HAL_SURFACE_ENTRY_INFO_ARRAY
{
    uint32_t maxEntryNum;
    uint32_t usedIndex;
    PCM_SURFACE_DETAILS surfEntryInfos;
    uint32_t globalSurfNum;
    PCM_SURFACE_DETAILS globalSurfInfos;
};

struct CM_HAL_SURFACE_ENTRY_INFO_ARRAYS
{
    uint32_t kernelNum;
    CM_HAL_SURFACE_ENTRY_INFO_ARRAY *surfEntryInfosArray;
};

//------------------------------------------------------------------------------
//| CM BARRIER MODES
//------------------------------------------------------------------------------
enum CM_BARRIER_MODE
{
    CM_NO_BARRIER           = 0,
    CM_LOCAL_BARRIER        = 1,
    CM_GLOBAL_BARRIER       = 2
};

struct CM_SAMPLER_BTI_ENTRY
{
    uint32_t samplerIndex;
    uint32_t samplerBTI;
};
typedef CM_SAMPLER_BTI_ENTRY *PCM_SAMPLER_BTI_ENTRY;

//*----------------
//| CM Query Type
//*----------------
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

//*-----------------------------------------------------------------------------
//| CM Query Caps
//*-----------------------------------------------------------------------------
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
        uint32_t surface2DCount;
        GMM_RESOURCE_FORMAT *surface2DFormats;
        CM_PLATFORM_INFO platformInfo;
    };
};
typedef CM_QUERY_CAPS *PCM_QUERY_CAPS;

//------------------------------------------------------------------------------
//| Enumeration for Task Status
//------------------------------------------------------------------------------
enum CM_HAL_TASK_STATUS
{
    CM_TASK_QUEUED,
    CM_TASK_IN_PROGRESS,
    CM_TASK_FINISHED,
    CM_TASK_RESET
};

//------------------------------------------------------------------------------
//| CM conditional batch buffer end information
//------------------------------------------------------------------------------
struct CM_HAL_CONDITIONAL_BB_END_INFO
{
    uint32_t bufferTableIndex;
    uint32_t offset;
    uint32_t compareValue;
    bool  disableCompareMask;
    bool  endCurrentLevel;
    uint32_t  operatorCode;
};
typedef CM_HAL_CONDITIONAL_BB_END_INFO *PCM_HAL_CONDITIONAL_BB_END_INFO;

//------------------------------------------------------------------------------
//| HAL CM Query Task Param
//------------------------------------------------------------------------------
struct CM_HAL_QUERY_TASK_PARAM
{
    int32_t taskId;                        // [in]  Task ID
    uint32_t taskType;                     // [in]  Task type
    CM_QUEUE_CREATE_OPTION queueOption;    // [in]  Queue type
    CM_HAL_TASK_STATUS status;             // [out] Task Status
    uint64_t taskDurationNs;               // [out] Task Duration
    uint64_t taskDurationTicks;            // [out] Task Duration in Ticks
    uint64_t taskHWStartTimeStampInTicks;  // [out] Task Start Time Stamp in Ticks
    uint64_t taskHWEndTimeStampInTicks;    // [out] Task End Time Stamp in Ticks
    LARGE_INTEGER taskGlobalSubmitTimeCpu; // [out] The CM task submission time in CPU
    LARGE_INTEGER taskSubmitTimeGpu;       // [out] The CM task submission time in GPU
    LARGE_INTEGER taskHWStartTimeStamp;    // [out] The task start execution time in GPU
    LARGE_INTEGER taskHWEndTimeStamp;      // [out] The task end execution time in GPU
};
typedef CM_HAL_QUERY_TASK_PARAM *PCM_HAL_QUERY_TASK_PARAM;

//*-----------------------------------------------------------------------------
//| Execute Group data params
//*-----------------------------------------------------------------------------
struct CM_HAL_EXEC_TASK_GROUP_PARAM
{
    PCM_HAL_KERNEL_PARAM *kernels;   // [in]  Array of Kernel data
    uint32_t *kernelSizes;         // [in]  Parallel array of Kernel Size
    uint32_t numKernels;             // [in]  Number of Kernels in a task
    int32_t  taskIdOut;              // [out] Task ID
    uint32_t threadSpaceWidth;       // [in]  thread space width within group
    uint32_t threadSpaceHeight;      // [in]  thread space height within group
    uint32_t threadSpaceDepth;       // [in]  thread space depth within group
    uint32_t groupSpaceWidth;        // [in]  group space width
    uint32_t groupSpaceHeight;       // [in]  group space height
    uint32_t groupSpaceDepth;        // [in]  group space depth
    uint32_t slmSize;                // [in]  SLM size per thread group in 1KB unit
    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS surEntryInfoArrays;  // [in]  GT-PIN
    void *osData;                    // [out] Used for Linux OS data to pass to event
    uint64_t syncBitmap;             // [in]  synchronization flag among kernels
    bool globalSurfaceUsed;          // [in]  is global surface used
    uint32_t *kernelCurbeOffset;   // [in]  Array of Kernel Curbe Offset
    bool kernelDebugEnabled;         // [in] kernel debug is enabled
    CM_TASK_CONFIG taskConfig;       // [in] task Config
    CM_EXECUTION_CONFIG krnExecCfg[CM_MAX_KERNELS_PER_TASK]; // [in] kernel execution config in a task. replace numOfWalkers in CM_TASK_CONFIG.
    void *userDefinedMediaState;     // [in] pointer to a user defined media state heap block
    CM_QUEUE_CREATE_OPTION queueOption;  // [in] multiple contexts queue option
    PMOS_VIRTUALENGINE_HINT_PARAMS mosVeHintParams; // [in] pointer to virtual engine paramter saved in CmQueueRT
    uint64_t conditionalEndBitmap;       // [in] bit map for conditional end b/w kernels
    CM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS];
};
typedef CM_HAL_EXEC_TASK_GROUP_PARAM *PCM_HAL_EXEC_GROUP_TASK_PARAM;

struct CM_HAL_EXEC_HINTS_TASK_PARAM
{
    PCM_HAL_KERNEL_PARAM *kernels;     // [in]  Array of kernel data
    uint32_t *kernelSizes;           // [in]  Parallel array of kernel size
    uint32_t numKernels;               // [in]  Number of kernels in a task
    int32_t taskIdOut;                 // [out] Task ID
    uint32_t hints;                    // [in]  Hints
    uint32_t numTasksGenerated;        // [in] Number of task generated already for split task
    bool isLastTask;                   // [in] Used to split tasks
    void *osData;                      // [out] Used for Linux OS data to pass to event
    uint32_t *kernelCurbeOffset;     // [in]  Kernel Curbe offset
    void *userDefinedMediaState;       // [in]  pointer to a user defined media state heap block
    CM_QUEUE_CREATE_OPTION queueOption;  // [in] multiple contexts queue option
};
typedef CM_HAL_EXEC_HINTS_TASK_PARAM *PCM_HAL_EXEC_HINTS_TASK_PARAM;

//------------------------------------------------------------------------------
//| CM scoreboard XY with color,mask and slice-sub-slice select
//------------------------------------------------------------------------------
struct CM_HAL_SCOREBOARD
{
    int32_t x;
    int32_t y;
    uint8_t mask;
    uint8_t resetMask;
    uint8_t color;
    uint8_t sliceSelect;
    uint8_t subSliceSelect;
};
typedef CM_HAL_SCOREBOARD *PCM_HAL_SCOREBOARD;

//------------------------------------------------------------------------------
//| CM scoreboard XY with mask and resetMask for Enqueue path
//------------------------------------------------------------------------------
struct CM_HAL_MASK_AND_RESET
{
    uint8_t mask;
    uint8_t resetMask;
};
typedef CM_HAL_MASK_AND_RESET *PCM_HAL_MASK_AND_RESET;

//------------------------------------------------------------------------------
//| CM dependency information
//------------------------------------------------------------------------------
struct CM_HAL_DEPENDENCY
{
    uint32_t count;
    int32_t deltaX[CM_HAL_MAX_DEPENDENCY_COUNT];
    int32_t deltaY[CM_HAL_MAX_DEPENDENCY_COUNT];
};

struct CM_HAL_EXEC_TASK_PARAM
{
    PCM_HAL_KERNEL_PARAM *kernels;  // [in]  Array of Kernel data
    uint32_t *kernelSizes;         // [in]  Parallel array of Kernel Size
    uint32_t numKernels;            // [in]  Number of Kernels in a task
    int32_t taskIdOut;              // [out] Task ID
    CM_HAL_SCOREBOARD **threadCoordinates;  // [in]  Scoreboard(x,y)
    CM_DEPENDENCY_PATTERN dependencyPattern;  // [in]  pattern
    uint32_t threadSpaceWidth;       // [in]  width
    uint32_t threadSpaceHeight;      // [in]  height
    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS surfEntryInfoArrays;  // [out] Used by GT-Pin
    void *osData;                    // [out] Used for Linux OS data to pass to event
    uint32_t colorCountMinusOne;                // [in]
    PCM_HAL_MASK_AND_RESET *dependencyMasks;  // [in]  media object thread dependency masks
    uint64_t syncBitmap;                      // [in] bit map for sync b/w kernels
    bool globalSurfaceUsed;                    // [in] if global surface used
    uint32_t *kernelCurbeOffset;              // [in]  array of kernel's curbe offset
    CM_WALKING_PATTERN walkingPattern;          // [in]  media walking pattern
    uint8_t walkingParamsValid;                 // [in] for engineering build
    CM_WALKING_PARAMETERS walkingParams;        // [in] for engineering build
    uint8_t dependencyVectorsValid;             // [in] for engineering build
    CM_HAL_DEPENDENCY dependencyVectors;        // [in] for engineering build
    CM_MW_GROUP_SELECT mediaWalkerGroupSelect;  // [in]
    bool kernelDebugEnabled;                   // [in] kernel debug is enabled
    uint64_t conditionalEndBitmap;            // [in] bit map for conditional end b/w kernels
    CM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS];
    CM_TASK_CONFIG taskConfig;                  // [in] task Config
    void *userDefinedMediaState;             // [in] pointer to a user defined media state heap block
    CM_QUEUE_CREATE_OPTION queueOption;         // [in] multiple contexts queue option
};
typedef CM_HAL_EXEC_TASK_PARAM *PCM_HAL_EXEC_TASK_PARAM;

//*-----------------------------------------------------------------------------
//| HAL CM Task Param
//*-----------------------------------------------------------------------------
struct CM_HAL_TASK_PARAM
{
    uint32_t numKernels;                    // [in] number of kernels
    uint64_t syncBitmap;                    // [in] Sync bitmap
    uint32_t batchBufferSize;               // [in] Size of Batch Buffer Needed
    uint32_t vfeCurbeSize;                  // [out] Sum of CURBE Size
    uint32_t urbEntrySize;                  // [out] Maximum Payload Size
    CM_HAL_SCOREBOARD **threadCoordinates;  // [in] Scoreboard(x,y)
    CM_DEPENDENCY_PATTERN dependencyPattern;  // [in] pattern
    uint32_t threadSpaceWidth;                // [in] width
    uint32_t threadSpaceHeight;               // [in] height
    uint32_t groupSpaceWidth;                 // [in] group space width
    uint32_t groupSpaceHeight;                // [in] group space height
    uint32_t slmSize;                         // [in] size of SLM
    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS surfEntryInfoArrays;  // [in] GTPin
    uint32_t curKernelIndex;
    uint32_t colorCountMinusOne;          // [in] color count
    PCM_HAL_MASK_AND_RESET *dependencyMasks;  // [in]  Thread dependency masks
    uint8_t reuseBBUpdateMask;            // [in] re-use batch buffer and just update mask
    uint32_t surfacePerBT;                // [out] surface number for binding table
    bool blGpGpuWalkerEnabled;            // [out]
    CM_WALKING_PATTERN walkingPattern;    // [in] media walking pattern
    bool hasBarrier;                      // [in] if there is barrier
    uint8_t walkingParamsValid;           // [in] for engineering build
    CM_WALKING_PARAMETERS walkingParams;  // [in] for engineering build
    uint8_t dependencyVectorsValid;       // [in] for engineering build
    CM_HAL_DEPENDENCY dependencyVectors;  // [in] for engineering build
    CM_MW_GROUP_SELECT mediaWalkerGroupSelect;  // [in]
    uint32_t kernelDebugEnabled;                // [in]
    uint64_t conditionalEndBitmap;            // [in] conditional end bitmap
    CM_HAL_CONDITIONAL_BB_END_INFO
    conditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS];  // [in] conditional BB end info used to fill conditionalBBEndParams

    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS
    conditionalBBEndParams[CM_MAX_CONDITIONAL_END_CMDS];

    CM_TASK_CONFIG taskConfig;       // [in] task Config
    CM_EXECUTION_CONFIG krnExecCfg[CM_MAX_KERNELS_PER_TASK]; // [in] kernel execution config in a task. replace numOfWalkers in CM_TASK_CONFIG.

    void *userDefinedMediaState;  // [in] pointer to a user defined media state heap block

    // [in] each kernel's sampler heap offset from the DSH sampler heap base
    unsigned int samplerOffsetsByKernel[CM_MAX_KERNELS_PER_TASK];

    // [in] each kernel's sampler count
    unsigned int samplerCountsByKernel[CM_MAX_KERNELS_PER_TASK];

    // [in] each kernel's indirect sampler heap offset from the DSH sampler heap base
    unsigned int samplerIndirectOffsetsByKernel[CM_MAX_KERNELS_PER_TASK];

    CM_QUEUE_CREATE_OPTION queueOption;         // [in] multiple contexts queue option
    PMOS_VIRTUALENGINE_HINT_PARAMS mosVeHintParams; // [in] pointer to virtual engine paramter saved in CmQueueRT
};
typedef CM_HAL_TASK_PARAM *PCM_HAL_TASK_PARAM;

//------------------------------------------------------------------------------
//| CM batch buffer dirty status
//------------------------------------------------------------------------------
enum CM_HAL_BB_DIRTY_STATUS
{
    CM_HAL_BB_CLEAN = 0,
    CM_HAL_BB_DIRTY = 1
};

//------------------------------------------------------------------------------
//| CM dispatch information for 26Z (for EnqueueWithHints)
//------------------------------------------------------------------------------
struct CM_HAL_WAVEFRONT26Z_DISPATCH_INFO
{
    uint32_t numWaves;
    uint32_t *numThreadsInWave;
};

//*-----------------------------------------------------------------------------
//| HAL CM Kernel Threadspace Param
//*-----------------------------------------------------------------------------
struct CM_HAL_KERNEL_THREADSPACE_PARAM
{
    uint16_t threadSpaceWidth;             // [in] Kernel Threadspace width
    uint16_t threadSpaceHeight;            // [in] Kernel Threadspace height
    CM_DEPENDENCY_PATTERN patternType;      // [in] Kernel dependency as enum
    CM_HAL_DEPENDENCY dependencyInfo;       // [in] Kernel dependency
    PCM_HAL_SCOREBOARD threadCoordinates;  // [in]
    uint8_t reuseBBUpdateMask;              // [in]
    CM_HAL_WAVEFRONT26Z_DISPATCH_INFO dispatchInfo;  // [in]
    uint8_t globalDependencyMask;           // [in] dependency mask in gloabal dependency vectors
    uint8_t walkingParamsValid;             // [in] for engineering build
    CM_WALKING_PARAMETERS walkingParams;    // [in] for engineering build
    uint8_t dependencyVectorsValid;         // [in] for engineering build
    CM_HAL_DEPENDENCY dependencyVectors;    // [in] for engineering build
    uint32_t colorCountMinusOne;            // [in] for color count minus one
    CM_MW_GROUP_SELECT groupSelect;         // [in] for group select on BDW+
    CM_HAL_BB_DIRTY_STATUS bbDirtyStatus;   // [in] batch buffer dirty status
    CM_WALKING_PATTERN walkingPattern;      // [in] media walking pattern as enum
};
typedef CM_HAL_KERNEL_THREADSPACE_PARAM *PCM_HAL_KERNEL_THREADSPACE_PARAM;

//------------------------------------------------------------------------------
//| CM buffer types
//------------------------------------------------------------------------------
enum CM_BUFFER_TYPE
{
    CM_BUFFER_N             = 0,
    CM_BUFFER_UP            = 1,
    CM_BUFFER_SVM           = 2,
    CM_BUFFER_GLOBAL        = 3,
    CM_BUFFER_STATE         = 4,
    CM_BUFFER_STATELESS     = 5
};

//------------------------------------------------------------------------------
//| CM shift direction. Used for CloneKernel API to adjust head kernel allocation ID.
//| Need to adjust IDs after the kernel allocation entries are shifted
//| i.e. neighboring free kernel allocation entries are combined into a single larger entry
//------------------------------------------------------------------------------
enum CM_SHIFT_DIRECTION
{
    CM_SHIFT_LEFT  = 0,
    CM_SHIFT_RIGHT = 1
};

//---------------
//| CM clone type
//---------------
enum CM_CLONE_TYPE
{
    CM_NO_CLONE = 0,             // regular kernel, not created from CloneKernel API and has no kernels that were cloned from it
    CM_CLONE_ENTRY = 1,          // 64B kernel allocation entry for a cloned kernel (will point to the head kernel's binary)
    CM_HEAD_KERNEL  = 2,         // kernel allocation entry that contains kernel binary (clone kernels will use this offset)
    CM_CLONE_AS_HEAD_KERNEL = 3  // cloned kernel is serving as a head kernel (original kernel and other clones can use this offset)
};

enum CM_STATE_BUFFER_TYPE
{
    CM_STATE_BUFFER_NONE = 0,
    CM_STATE_BUFFER_CURBE = 1,
};

//*-----------------------------------------------------------------------------
//| Enumeration for Kernel argument type
//*-----------------------------------------------------------------------------
enum CM_HAL_KERNEL_ARG_KIND
{
    CM_ARGUMENT_GENERAL            = 0x0,
    CM_ARGUMENT_SAMPLER            = 0x1,
    CM_ARGUMENT_SURFACE2D          = 0x2,
    CM_ARGUMENT_SURFACEBUFFER      = 0x3,
    CM_ARGUMENT_SURFACE3D          = 0x4,
    CM_ARGUMENT_SURFACE_VME        = 0x5,
    CM_ARGUMENT_VME_STATE          = 0x6,
    CM_ARGUMENT_SURFACE2D_UP       = 0x7,
    CM_ARGUMENT_SURFACE_SAMPLER8X8_AVS = 0x8,
    CM_ARGUMENT_SURFACE_SAMPLER8X8_VA = 0x9,
    CM_ARGUMENT_SURFACE2D_SAMPLER  = 0xb,
    CM_ARGUMENT_SURFACE            = 0xc,
    CM_ARGUMENT_SURFACE2DUP_SAMPLER= 0xd,
    CM_ARGUMENT_IMPLICT_LOCALSIZE = 0xe,
    CM_ARGUMENT_IMPLICT_GROUPSIZE = 0xf,
    CM_ARGUMENT_IMPLICIT_LOCALID = 0x10,
    CM_ARGUMENT_STATE_BUFFER       = 0x11,
    CM_ARGUMENT_GENERAL_DEPVEC       = 0x20,
    CM_ARGUMENT_SURFACE2D_SCOREBOARD = 0x2A  //used for SW scoreboarding
};

//*-----------------------------------------------------------------------------
//| HAL CM Kernel Argument Param
//*-----------------------------------------------------------------------------
struct CM_HAL_KERNEL_ARG_PARAM
{
    CM_HAL_KERNEL_ARG_KIND kind;  // [in] Kind of argument
    uint32_t unitCount;          // [in] 1 if argument is kernel arg, otherwise equal to thread count
    uint32_t unitSize;       // [in] Unit Size of the argument
    uint32_t payloadOffset;  // [in] Offset to Thread Payload
    bool perThread;          // [in] Per kernel / per thread argument
    uint8_t *firstValue;     // [in] Byte Pointer to First Value.
    uint32_t nCustomValue;    // [in] CM defined value for the special kind of argument
    uint32_t aliasIndex;     // [in] Alias index, used for CmSurface2D alias
    bool aliasCreated;       // [in] Whether or not alias was created for this argument
    bool isNull;             // [in] Whether this argument is a null surface
};
typedef CM_HAL_KERNEL_ARG_PARAM *PCM_HAL_KERNEL_ARG_PARAM;

//*-----------------------------------------------------------------------------
//| HAL CM Sampler BTI Entry
//*-----------------------------------------------------------------------------
struct CM_HAL_SAMPLER_BTI_ENTRY
{
    uint32_t samplerIndex;
    uint32_t samplerBTI;
};
typedef CM_HAL_SAMPLER_BTI_ENTRY *PCM_HAL_SAMPLER_BTI_ENTRY;

//*-----------------------------------------------------------------------------
//| HAL CM Sampler BTI Param
//*-----------------------------------------------------------------------------
struct CM_HAL_SAMPLER_BTI_PARAM
{
    CM_HAL_SAMPLER_BTI_ENTRY samplerInfo[ CM_MAX_SAMPLER_TABLE_SIZE ];
    uint32_t samplerCount;
};
typedef CM_HAL_SAMPLER_BTI_PARAM *PCM_HAL_SAMPLER_BTI_PARAM;

struct CM_HAL_CLONED_KERNEL_PARAM
{
    bool isClonedKernel;
    int32_t kernelID;
    bool hasClones;
};

struct CM_GPGPU_WALKER_PARAMS
{
    uint32_t interfaceDescriptorOffset : 5;
    uint32_t gpgpuEnabled              : 1;
    uint32_t                           : 26;
    uint32_t threadWidth;
    uint32_t threadHeight;
    uint32_t threadDepth;
    uint32_t groupWidth;
    uint32_t groupHeight;
    uint32_t groupDepth;
};
typedef CM_GPGPU_WALKER_PARAMS *PCM_GPGPU_WALKER_PARAMS;

struct CM_SAMPLER_STATISTICS
{
    uint32_t samplerCount[MAX_ELEMENT_TYPE_COUNT];
    uint32_t samplerMultiplier[MAX_ELEMENT_TYPE_COUNT];  //used for distinguishing whether need to take two
    uint32_t samplerIndexBase[MAX_ELEMENT_TYPE_COUNT];
};

//*-----------------------------------------------------------------------------
//| HAL CM Kernel Param
//*-----------------------------------------------------------------------------
struct CM_HAL_KERNEL_PARAM
{
    CM_HAL_KERNEL_ARG_PARAM argParams[CM_MAX_ARGS_PER_KERNEL];
    CM_SAMPLER_STATISTICS samplerStatistics;  // [in] each sampler element type count in the kernel argument
    uint8_t *kernelData;            // [in] Pointer to Kernel data
    uint32_t kernelDataSize;        // [in] Size of Kernel Data
    uint8_t *movInsData;            // [in] pointer to move instruction data
    uint32_t movInsDataSize;        // [in] size of move instructions
    uint8_t *kernelBinary;          // [in] Execution code for the kernel
    uint32_t kernelBinarySize;      // [in] Size of Kernel Binary
    uint32_t numThreads;            // [in] Number of threads
    uint32_t numArgs;               // [in] Number of Kernel Args
    bool perThreadArgExisted;
    uint32_t numSurfaces;           // [in] Number of Surfaces used in this kernel
    uint32_t payloadSize;           // [in] Kernel Payload Size
    uint32_t totalCurbeSize;        // [in] total CURBE size, GPGPU
    uint32_t curbeOffset;           // [in] curbe offset of kernel
    uint32_t curbeSizePerThread;    // [in] CURBE size per thread
    uint32_t crossThreadConstDataLen;    // [in] Cross-thread constant data length HSW+
    uint32_t barrierMode;           // [in] Barrier mode, 0-No barrier, 1-local barrier, 2-global barrier
    uint32_t numberThreadsInGroup;  // [in] Number of Threads in Thread Group
    uint32_t slmSize;               // [in] SLM size in 1K-Bytes or 4K-Bytes
    uint32_t spillSize;             // [in] Kernel spill area, obtained from JITTER
    uint32_t cmFlags;              // [in] Kernel flags
    uint64_t kernelId;             // [in] Kernel Id
    CM_HAL_KERNEL_THREADSPACE_PARAM kernelThreadSpaceParam;  // [in] ThreadSpace Information
    CM_HAL_WALKER_PARAMS walkerParams;  // [out] Media walker parameters for kernel:filled in HalCm_ParseTask
    bool globalSurfaceUsed;         // [in] Global surface used
    uint32_t globalSurface[CM_MAX_GLOBAL_SURFACE_NUMBER];  // [in] Global Surface indexes
    CM_GPGPU_WALKER_PARAMS gpgpuWalkerParams;
    bool kernelDebugEnabled;        // [in] kernel debug is enabled
    CM_HAL_INDIRECT_DATA_PARAM indirectDataParam;
    char kernelName[ CM_MAX_KERNEL_NAME_SIZE_IN_BYTE ];  // [in] A fixed size array to hold the kernel name
    CM_HAL_SAMPLER_BTI_PARAM samplerBTIParam;
    uint32_t localIdIndex;           //local ID index has different location with different compiler version
    CM_HAL_CLONED_KERNEL_PARAM clonedKernelParam;
    CM_STATE_BUFFER_TYPE stateBufferType;
    std::list<SamplerParam> *samplerHeap;
};
typedef CM_HAL_KERNEL_PARAM *PCM_HAL_KERNEL_PARAM;

//*----------------------
//| CM Set Type
//*----------------------
enum CM_SET_TYPE
{
    CM_SET_MAX_HW_THREADS,
    CM_SET_HW_L3_CONFIG
};

struct CM_HAL_MAX_SET_CAPS_PARAM
{
    CM_SET_TYPE type;
    union
    {
        uint32_t maxValue;
        L3ConfigRegisterValues l3CacheValues;
    };

};
typedef CM_HAL_MAX_SET_CAPS_PARAM *PCM_HAL_MAX_SET_CAPS_PARAM;

//------------------------------------------------------------------------------
//| CM Buffer Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_BUFFER_PARAM
{
    size_t                      size;                                          // [in]         Buffer Size
    CM_BUFFER_TYPE              type;                                          // [in]         Buffer type: Buffer, UP, SVM
    void                        *data;                                         // [in/out]     System address of this buffer
    uint32_t                    handle;                                        // [in/out]     Handle
    uint32_t                    lockFlag;                                      // [in]         Lock flag
    PMOS_RESOURCE               mosResource;                                   // [in]         Mos resource
    bool                        isAllocatedbyCmrtUmd;                          // [in]         Flag for Cmrt@umd Created Buffer
    uint64_t                    gfxAddress;                                    // [out]        GFX address of this buffer
} CM_HAL_BUFFER_PARAM, *PCM_HAL_BUFFER_PARAM;

//------------------------------------------------------------------------------
//| CM Buffer Set Surface State Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_BUFFER_SURFACE_STATE_PARAM
{
    size_t                      size;                                          // [in]         Surface State Size
    uint32_t                    offset;                                        // [in]         Surface State Base Address Offset
    uint16_t                    mocs;                                          // [in]         Surface State mocs settings
    uint32_t                    aliasIndex;                                    // [in]         Surface Alias Index
    uint32_t                    handle;                                       // [in]         Handle
} CM_HAL_BUFFER_SURFACE_STATE_PARAM, *PCM_HAL_BUFFER_SURFACE_STATE_PARAM;

//------------------------------------------------------------------------------
//| CM BB Args
//------------------------------------------------------------------------------
typedef struct _CM_HAL_BB_ARGS
{
    uint64_t  kernelIds[CM_MAX_KERNELS_PER_TASK];
    uint64_t  refCount;
    bool      latest;
} CM_HAL_BB_ARGS, *PCM_HAL_BB_ARGS;

//------------------------------------------------------------------------------
//| CM 2DUP Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_UP_PARAM
{
    uint32_t                    width;                                         // [in]         Surface Width
    uint32_t                    height;                                        // [in]         Surface Height
    MOS_FORMAT                  format;                                         // [in]         Surface Format
    void                        *data;                                          // [in/out]     Pointer to data
    uint32_t                    pitch;                                         // [out]        Pitch
    uint32_t                    physicalSize;                                  // [out]        Physical size
    uint32_t                    handle;                                       // [in/out]     Handle
} CM_HAL_SURFACE2D_UP_PARAM, *PCM_HAL_SURFACE2D_UP_PARAM;

//------------------------------------------------------------------------------
//| CM 2D Get Surface Information Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_INFO_PARAM
{
    uint32_t                    width;                                         // [out]         Surface Width
    uint32_t                    height;                                        // [out]         Surface Height
    MOS_FORMAT                  format;                                         // [out]         Surface Format
    uint32_t                    pitch;                                         // [out]         Pitch
    UMD_RESOURCE                surfaceHandle ;                                 // [in]          Driver Handler
    uint32_t                    surfaceAllocationIndex;                         // [in]          KMD Driver Handler
} CM_HAL_SURFACE2D_INFO_PARAM, *PCM_HAL_SURFACE2D_INFO_PARAM;

//------------------------------------------------------------------------------
//| CM 2D Set Surface State Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_SURFACE_STATE_PARAM
{
    uint32_t format;
    uint32_t width;
    uint32_t height;
    uint32_t depth;
    uint32_t pitch;
    uint16_t memoryObjectControl;
    uint32_t surfaceXOffset;
    uint32_t surfaceYOffset;
    uint32_t surfaceOffset;
} CM_HAL_SURFACE2D_SURFACE_STATE_PARAM, *PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM;

//------------------------------------------------------------------------------
//| CM 2D Lock/Unlock Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM
{
    uint32_t                    width;                                       // [in]         Surface Width
    uint32_t                    height;                                      // [in]         Surface Height
    uint32_t                    size;                                        // [in]         Surface total size
    MOS_FORMAT                  format;                                      // [in]         Surface Format
    void                        *data;                                       // [in/out]     Pointer to data
    uint32_t                    pitch;                                       // [out]        Pitch
    MOS_PLANE_OFFSET            YSurfaceOffset;                              // [out]        Y plane Offset
    MOS_PLANE_OFFSET            USurfaceOffset;                              // [out]        U plane Offset
    MOS_PLANE_OFFSET            VSurfaceOffset;                              // [out]        V plane Offset
    uint32_t                    lockFlag;                                    // [out]        lock flag
    uint32_t                    handle;                                      // [in/out]     Handle
    bool                        useGmmOffset;                                // [in/out]     Only use Gmm offset in Linux
} CM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM, *PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM;

//*-----------------------------------------------------------------------------
//| Compression state
//*-----------------------------------------------------------------------------
enum MEMCOMP_STATE
{
    MEMCOMP_DISABLED = 0,
    MEMCOMP_HORIZONTAL,
    MEMCOMP_VERTICAL
};

//------------------------------------------------------------------------------
//|  CM 2D Surface Compression Parameter
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_COMPRESSION_PARAM
{
    uint32_t                    handle;
    MEMCOMP_STATE               mmcMode;
}CM_HAL_SURFACE2D_COMPRESSIOM_PARAM, *PCM_HAL_SURFACE2D_COMPRESSION_PARAM;

//------------------------------------------------------------------------------
//| CM 2D Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SURFACE2D_PARAM
{
    uint32_t                    isAllocatedbyCmrtUmd;                          // [in]         Flag for Cmrt@umd Created Surface
    PMOS_RESOURCE               mosResource;                                   // [in]         Mos resource
    uint32_t                    width;                                         // [in]         Surface Width
    uint32_t                    height;                                        // [in]         Surface Height
    MOS_FORMAT                  format;                                        // [in]         Surface Format
    void                        *data;                                         // [in]         PData
    uint32_t                    pitch;                                         // [out]        Pitch
    uint32_t                    handle;                                        // [in/out]     Handle
} CM_HAL_SURFACE2D_PARAM, *PCM_HAL_SURFACE2D_PARAM;

//------------------------------------------------------------------------------
//| CM 3D Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_3DRESOURCE_PARAM
{
    uint32_t                    height;                                        // [in]       Surface Height
    uint32_t                    width;                                         // [in]       Surface Width
    uint32_t                    depth;                                         // [in]       Surface Depth
    MOS_FORMAT                  format;
    void                        *data;                                         // [in/out]   Pointer to data
    uint32_t                    handle;                                        // [in/out]   Handle
    uint32_t                    lockFlag;                                      // [in]       Lock flag
    uint32_t                    pitch;                                         // [out]      Pitch of Resource
    uint32_t                    qpitch;                                        // [out]      QPitch of the Resource
    bool                        qpitchEnabled;                                  // [out]      if QPitch is supported by hw
} CM_HAL_3DRESOURCE_PARAM, *PCM_HAL_3DRESOURCE_PARAM;

//------------------------------------------------------------------------------
//| HalCm Kernel Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_SETUP
{
    RENDERHAL_KERNEL_PARAM  renderParam;
    MHW_KERNEL_PARAM        cacheEntry;
} CM_HAL_KERNEL_SETUP, *PCM_HAL_KERNEL_SETUP;

//------------------------------------------------------------------------------
//| CM vebox settings
//------------------------------------------------------------------------------
typedef struct _CM_VEBOX_SETTINGS
{
    bool    diEnabled;
    bool    dndiFirstFrame;
    bool    iecpEnabled;
    bool    dnEnabled;
    uint32_t diOutputFrames;
    bool    demosaicEnabled;
    bool    vignetteEnabled;
    bool    hotPixelFilterEnabled;
}CM_VEBOX_SETTINGS;

#define VEBOX_SURFACE_NUMBER                 (16)     //MAX

#define CM_VEBOX_PARAM_PAGE_SIZE   0x1000
#define CM_VEBOX_PARAM_PAGE_NUM    5

typedef struct _CM_AVS_TABLE_STATE_PARAMS {
    bool               bypassXAF;
    bool               bypassYAF;
    uint8_t            defaultSharpLevel;
    uint8_t            maxDerivative4Pixels;
    uint8_t            maxDerivative8Pixels;
    uint8_t            transitionArea4Pixels;
    uint8_t            transitionArea8Pixels;
    CM_AVS_COEFF_TABLE tbl0X[ NUM_POLYPHASE_TABLES ];
    CM_AVS_COEFF_TABLE tbl0Y[ NUM_POLYPHASE_TABLES ];
    CM_AVS_COEFF_TABLE tbl1X[ NUM_POLYPHASE_TABLES ];
    CM_AVS_COEFF_TABLE tbl1Y[ NUM_POLYPHASE_TABLES ];
    bool               enableRgbAdaptive;
    bool               adaptiveFilterAllChannels;
} CM_AVS_TABLE_STATE_PARAMS, *PCM_AVS_TABLE_STATE_PARAMS;

typedef  struct _CM_HAL_AVS_PARAM {
    MHW_SAMPLER_STATE_AVS_PARAM avsState;             // [in] avs state table.
    CM_AVS_TABLE_STATE_PARAMS   avsTable;             // [in] avs table.
} CM_HAL_AVS_PARAM, *PCM_HAL_AVS_PARAM;

/*
 *  CONVOLVE STATE DATA STRUCTURES
 */
typedef struct _CM_HAL_CONVOLVE_COEFF_TABLE{
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
}CM_HAL_CONVOLVE_COEFF_TABLE;

#define CM_NUM_CONVOLVE_ROWS_SKL 31
typedef struct _CM_HAL_CONVOLVE_STATE_MSG{
        bool coeffSize; //true 16-bit, false 8-bit
        uint8_t scaleDownValue; //Scale down value
        uint8_t width; //Kernel Width
        uint8_t height; //Kernel Height
        //SKL mode
        bool isVertical32Mode;
        bool isHorizontal32Mode;
        bool sklMode;  // new added
        CM_CONVOLVE_SKL_TYPE nConvolveType;
        CM_HAL_CONVOLVE_COEFF_TABLE table[ CM_NUM_CONVOLVE_ROWS_SKL ];
} CM_HAL_CONVOLVE_STATE_MSG;

/*
 *   MISC SAMPLER8x8 State
 */
typedef struct _CM_HAL_MISC_STATE {
    //uint32_t 0
    union{
        struct{
            uint32_t Height    : 4;
            uint32_t Width     : 4;
            uint32_t Reserved  : 8;
            uint32_t Row0      : 16;
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
} CM_HAL_MISC_STATE;

typedef struct _CM_HAL_SAMPLER_8X8_STATE
{
    CM_HAL_SAMPLER_8X8_TYPE         stateType;             // [in] types of Sampler8x8
    union {
        CM_HAL_AVS_PARAM            avsParam;             // [in] avs parameters.
        CM_HAL_CONVOLVE_STATE_MSG   convolveState;
        CM_HAL_MISC_STATE           miscState;
        //Will add other sampler8x8 types later.
    };
} CM_HAL_SAMPLER_8X8_STATE;

typedef struct _CM_HAL_SAMPLER_8X8_TABLE
{
    CM_HAL_SAMPLER_8X8_TYPE         stateType;               // [in] types of Sampler8x8
    MHW_SAMPLER_AVS_TABLE_PARAM     mhwSamplerAvsTableParam; // [in] Sampler8x8 avs table
} CM_HAL_SAMPLER_8X8_TABLE, *PCM_HAL_SAMPLER_8X8_TABLE;

typedef struct _CM_HAL_SAMPLER_8X8_PARAM
{
    CM_HAL_SAMPLER_8X8_STATE          sampler8x8State;      // [in]  Sampler8x8 states
    uint32_t                          handle;             // [out] Handle
} CM_HAL_SAMPLER_8X8_PARAM, *PCM_HAL_SAMPLER_8X8_PARAM;

typedef struct _CM_HAL_SAMPLER_8X8_ENTRY{
    CM_HAL_SAMPLER_8X8_TABLE       sampler8x8State;       // [in]  Sampler8x8 states
    bool                           inUse;                // [out] If the entry has been occupied.
} CM_HAL_SAMPLER_8X8_ENTRY, *PCM_HAL_SAMPLER_8X8_ENTRY;

typedef struct _CM_HAL_BUFFER_SURFACE_STATE_ENTRY
{
    size_t surfaceStateSize;
    uint32_t surfaceStateOffset;
    uint16_t surfaceStateMOCS;
} CM_HAL_BUFFER_SURFACE_STATE_ENTRY, *PCM_HAL_BUFFER_SURFACE_STATE_ENTRY;

//------------------------------------------------------------------------------
//| HAL CM Buffer Table
//------------------------------------------------------------------------------
class CmSurfaceStateBufferMgr;
typedef struct _CM_HAL_BUFFER_ENTRY
{
    MOS_RESOURCE                        osResource;                                         // [in] Pointer to OS Resource
    size_t                              size;                                              // [in] Size of Buffer
    void                                *address;                                           // [in] SVM address
    void                                *gmmResourceInfo;                                   // [out] GMM resource info
    bool                                isAllocatedbyCmrtUmd;                               // [in] Whether Surface allocated by CMRT
    uint16_t                            memObjCtl;                                          // [in] MOCS value set from CMRT
    CM_HAL_BUFFER_SURFACE_STATE_ENTRY   surfaceStateEntry[CM_HAL_MAX_NUM_BUFFER_ALIASES];   // [in] width/height of surface to be used in surface state
    CmSurfaceStateBufferMgr             *surfStateMgr;
    bool                                surfStateSet;
} CM_HAL_BUFFER_ENTRY, *PCM_HAL_BUFFER_ENTRY;

//------------------------------------------------------------------------------
//| HAL CM 2D UP Table
//------------------------------------------------------------------------------
class CmSurfaceState2Dor3DMgr;
typedef struct _CM_HAL_SURFACE2D_UP_ENTRY
{
    MOS_RESOURCE                osResource;                                     // [in] Pointer to OS Resource
    uint32_t                    width;                                         // [in] Width of Surface
    uint32_t                    height;                                        // [in] Height of Surface
    MOS_FORMAT                  format;                                         // [in] Format of Surface
    void                        *gmmResourceInfo;                               // [out] GMM resource info
    uint16_t                    memObjCtl;                                      // [in] MOCS value set from CMRT
    CmSurfaceState2Dor3DMgr     *surfStateMgr;
} CM_HAL_SURFACE2D_UP_ENTRY, *PCM_HAL_SURFACE2D_UP_ENTRY;

typedef struct _CM_HAL_SURFACE_STATE_ENTRY
{
    uint32_t surfaceStateWidth;
    uint32_t surfaceStateHeight;
} CM_HAL_SURFACE_STATE_ENTRY, *PCM_HAL_SURFACE_STATE_ENTRY;

typedef struct _CM_HAL_VME_ARG_VALUE
{
    uint32_t                    fwRefNum;
    uint32_t                    bwRefNum;
    CM_HAL_SURFACE_STATE_ENTRY  surfStateParam;
    uint32_t                    curSurface;
    // IMPORTANT: in realization, this struct is always followed by ref surfaces array, fw followed by bw
    // Please use CmSurfaceVme::GetVmeCmArgSize() to allocate the memory for this structure
}CM_HAL_VME_ARG_VALUE, *PCM_HAL_VME_ARG_VALUE;

inline uint32_t *findRefInVmeArg(PCM_HAL_VME_ARG_VALUE value)
{
    return (uint32_t *)(value+1);
}

inline uint32_t *findFwRefInVmeArg(PCM_HAL_VME_ARG_VALUE value)
{
    return (uint32_t *)(value+1);
}

inline uint32_t *findBwRefInVmeArg(PCM_HAL_VME_ARG_VALUE value)
{
    return &((uint32_t *)(value+1))[value->fwRefNum];
}

inline uint32_t getVmeArgValueSize(PCM_HAL_VME_ARG_VALUE value)
{
    return sizeof(CM_HAL_VME_ARG_VALUE) + (value->fwRefNum + value->bwRefNum) * sizeof(uint32_t);
}

inline uint32_t getSurfNumFromArgArraySize(uint32_t argArraySize, uint32_t argNum)
{
    return (argArraySize - argNum*sizeof(CM_HAL_VME_ARG_VALUE))/sizeof(uint32_t) + argNum; // substract the overhead of the structure to get number of references, then add the currenct surface
}

//------------------------------------------------------------------------------
//| HAL CM 2D Table
//------------------------------------------------------------------------------
class CmSurfaceState2Dor3DMgr;
typedef struct _CM_HAL_SURFACE2D_ENTRY
{
    MOS_RESOURCE                osResource;                                    // [in] Pointer to OS Resource
    uint32_t                    width;                                         // [in] Width of Surface
    uint32_t                    height;                                        // [in] Height of Surface
    MOS_FORMAT                  format;                                         // [in] Format of Surface
    void                        *gmmResourceInfo;                               // [out] GMM resource info
    uint32_t                    isAllocatedbyCmrtUmd;                           // [in] Whether Surface allocated by CMRT
    uint32_t                    surfaceStateWidth;                             // [in] Width of Surface to be set in surface state
    uint32_t                    surfaceStateHeight;                            // [in] Height of Surface to be set in surface state
    bool                        readSyncs[MOS_GPU_CONTEXT_MAX];              // [in] Used in on demand sync for each gpu context
    CM_HAL_SURFACE2D_SURFACE_STATE_PARAM  surfaceStateParam[CM_HAL_MAX_NUM_2D_ALIASES];   // [in] width/height of surface to be used in surface state
    MHW_ROTATION                rotationFlag;
    int32_t                     chromaSiting;
    CM_FRAME_TYPE               frameType;
    uint16_t                    memObjCtl;                                      // [in] MOCS value set from CMRT
    CmSurfaceState2Dor3DMgr     *surfStateMgr;
    bool                        surfStateSet;
} CM_HAL_SURFACE2D_ENTRY, *PCM_HAL_SURFACE2D_ENTRY;

//------------------------------------------------------------------------------
//| HAL CM Buffer Table
//------------------------------------------------------------------------------
typedef struct _CM_HAL_3DRESOURCE_ENTRY
{
    MOS_RESOURCE           osResource;                                    // [in] Pointer to OS Resource
    uint32_t               width;                                         // [in] Width of Surface
    uint32_t               height;                                        // [in] Height of Surface
    uint32_t               depth;                                         // [in] Depth of Surface
    MOS_FORMAT             format;                                        // [in] Format of Surface
    uint16_t               memObjCtl;                                     // [in] MOCS value set from CMRT
    CmSurfaceState2Dor3DMgr *surfStateMgr;
} CM_HAL_3DRESOURCE_ENTRY, *PCM_HAL_3DRESOURCE_ENTRY;

//*-----------------------------------------------------------------------------
//| TimeStamp Resource. Used for storing task begin and end timestamps
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_TS_RESOURCE
{
    MOS_RESOURCE                osResource;                                     // [in] OS Resource
    bool                        locked;                                        // [in] Locked Flag
    uint8_t                     *data;                                          // [in] Linear Data
} CM_HAL_TS_RESOURCE, *PCM_HAL_TS_RESOURCE;

class FrameTrackerProducer;
struct CM_HAL_HEAP_PARAM
{
    uint32_t initialSizeGSH;
    uint32_t extendSizeGSH;
    FrameTrackerProducer *trackerProducer;
    HeapManager::Behavior behaviorGSH;
};

//------------------------------------------------------------------------------
//| HAL CM Struct for a multiple usage mapping from OSResource to BTI states
//------------------------------------------------------------------------------
typedef struct _CM_HAL_MULTI_USE_BTI_ENTRY
{
    union
    {
    struct
    {
        uint32_t regularSurfIndex : 8;
        uint32_t samplerSurfIndex : 8;
        uint32_t vmeSurfIndex : 8;
        uint32_t sampler8x8SurfIndex : 8;
    };
    struct
    {
        uint32_t value;
    };
    } BTI;

    struct
    {
        void  *regularBtiEntryPosition;
        void  *samplerBtiEntryPosition;
        void  *vmeBtiEntryPosition;
        void  *sampler8x8BtiEntryPosition;
    } BTITableEntry;
    uint32_t nPlaneNumber;
} CM_HAL_MULTI_USE_BTI_ENTRY, *PCM_HAL_MULTI_USE_BTI_ENTRY;

//------------------------------------------------------------------------------
//| HAL CM Struct for a table entry for state buffer
//------------------------------------------------------------------------------
typedef struct _CM_HAL_STATE_BUFFER_ENTRY
{
    void                    *kernelPtr;
    uint32_t                stateBufferIndex;
    CM_STATE_BUFFER_TYPE    stateBufferType;
    uint32_t                stateBufferSize;
    uint64_t                stateBufferVaPtr;
    PRENDERHAL_MEDIA_STATE  mediaStatePtr;
} CM_HAL_STATE_BUFFER_ENTRY;

typedef struct _CM_HAL_STATE *PCM_HAL_STATE;

//------------------------------------------------------------------------------
//| HAL CM Struct for a L3 settings
//------------------------------------------------------------------------------
typedef struct CmHalL3Settings
{
    bool    enableSlm;     // Enable SLM cache configuration
    bool    overrideSettings;      // Override cache settings
                                   // Override values
    bool    l3CachingEnabled;

    bool    cntlRegOverride;
    bool    cntlReg2Override;
    bool    cntlReg3Override;
    bool    sqcReg1Override;
    bool    sqcReg4Override;
    bool    lra1RegOverride;
    bool    tcCntlRegOverride;
    bool    allocRegOverride;

    unsigned long   cntlReg;
    unsigned long   cntlReg2;
    unsigned long   cntlReg3;
    unsigned long   sqcReg1;
    unsigned long   sqcReg4;
    unsigned long   lra1Reg;
    unsigned long   tcCntlReg;
    unsigned long   allocReg;
} *PCmHalL3Settings;

//------------------------------------------------------------------------------
//| HAL CM Device Param
//------------------------------------------------------------------------------
struct CM_HAL_DEVICE_PARAM
{
    uint32_t maxTasks;                      // [in] Max Tasks
    uint32_t maxKernelsPerTask;             // [in] Maximum Number of Kernels Per Task
    uint32_t maxKernelBinarySize;           // [in] Maximum binary size of the kernel
    uint32_t maxSamplerTableSize;           // [in] Max sampler table size
    uint32_t maxBufferTableSize;            // [in] Buffer table Size
    uint32_t max2DSurfaceUPTableSize;       // [in] 2D surfaceUP table Size
    uint32_t max2DSurfaceTableSize;         // [in] 2D surface table Size
    uint32_t max3DSurfaceTableSize;         // [in] 3D table Size
    uint32_t maxSampler8x8TableSize;        // [in] Max Sampler 8x8 table size
    uint32_t maxPerThreadScratchSpaceSize;  // [in] Max per hw thread scratch space size
    uint32_t maxAvsSamplers;                // [in] Max Number of AVS Samplers
    int32_t maxGshKernelEntries;            // [in] Max number of kernel entries in GSH
};
typedef CM_HAL_DEVICE_PARAM *PCM_HAL_DEVICE_PARAM;

//------------------------------------------------------------------------------
//| CM max parallelism information (for EnqueueWithHints)
//------------------------------------------------------------------------------
struct CM_HAL_PARALLELISM_GRAPH_INFO
{
    uint32_t maxParallelism;
    uint32_t numMaxRepeat;
    uint32_t numSteps;
};
typedef CM_HAL_PARALLELISM_GRAPH_INFO *PCM_HAL_PARALLELISM_GRAPH_INFO;

//----------------------------------------------------
//| CM kernel group information (for EnqueueWithHints)
//----------------------------------------------------
struct CM_HAL_KERNEL_GROUP_INFO
{
    uint32_t numKernelsFinished;
    uint32_t numKernelsInGroup;
    uint32_t groupFinished;
    uint32_t numStepsInGrp;
    uint32_t freqDispatch;
};
typedef CM_HAL_KERNEL_GROUP_INFO *PCM_HAL_KERNEL_GROUP_INFO;

//------------------------------------------------------------------------------
//| CM max hardware threads
//------------------------------------------------------------------------------
struct CM_HAL_MAX_HW_THREAD_VALUES
{
    uint32_t userFeatureValue;
    uint32_t apiValue;
};

struct CM_HAL_EXEC_VEBOX_TASK_PARAM;
typedef CM_HAL_EXEC_VEBOX_TASK_PARAM *PCM_HAL_EXEC_VEBOX_TASK_PARAM;

//------------------------------------------------------------------------------
//| HAL CM Register DmaCompleteEvent Handle Param
//------------------------------------------------------------------------------
struct CM_HAL_OSSYNC_PARAM
{
    HANDLE osSyncEvent;  //BB complete Notification
};
typedef CM_HAL_OSSYNC_PARAM *PCM_HAL_OSSYNC_PARAM;

struct CM_HAL_TASK_TIMESTAMP
{
    LARGE_INTEGER submitTimeInCpu[CM_MAXIMUM_TASKS];  // [out] The CM task submission time in CPU
    uint64_t submitTimeInGpu[CM_MAXIMUM_TASKS];        // [out] The CM task submission time in GPU
};
typedef CM_HAL_TASK_TIMESTAMP *PCM_HAL_TASK_TIMESTAMP;

struct CM_HAL_HINT_TASK_INDEXES
{
    uint32_t kernelIndexes[CM_MAX_TASKS_EU_SATURATION];    // [in/out] kernel indexes used for EU saturation
    uint32_t dispatchIndexes[CM_MAX_TASKS_EU_SATURATION];  // [in/out] dispatch indexes used for EU saturation
};

//-------------------------------
//| CM HW GT system info
//-------------------------------
struct CM_GT_SYSTEM_INFO
{
    uint32_t numMaxSlicesSupported;
    uint32_t numMaxSubSlicesSupported;
    GT_SLICE_INFO sliceInfo[GT_MAX_SLICE];
    bool isSliceInfoValid;
};
typedef CM_GT_SYSTEM_INFO *PCM_GT_SYSTEM_INFO;

//------------------------------------------------------------------------------
//| HAL CM State
//------------------------------------------------------------------------------
class CmExecutionAdv;
typedef struct _CM_HAL_STATE
{
    // Internal/private structures
    PLATFORM                    platform;
    MEDIA_FEATURE_TABLE         *skuTable;
    MEDIA_WA_TABLE              *waTable;
    PMOS_INTERFACE              osInterface;                                   // OS Interface                                 [*]
    PRENDERHAL_INTERFACE        renderHal;                                     // Render Engine Interface                      [*]
    MhwVeboxInterface           *veboxInterface;                               // Vebox Interface
    MhwCpInterface*             cpInterface;                                   // Cp  Interface
    PMHW_BATCH_BUFFER           batchBuffers;                                  // Array of Batch Buffers                       [*]
    PCM_HAL_TASK_PARAM          taskParam;                                     // Pointer to Task Param                        [*]
    PCM_HAL_TASK_TIMESTAMP      taskTimeStamp;                                 // Pointer to Task Time Stamp
    CM_HAL_TS_RESOURCE          renderTimeStampResource;                              // Resource to store timestamps                 [*]
    CM_HAL_TS_RESOURCE          veboxTimeStampResource;                               // Resource to store timestamps                 [*]
    CM_HAL_TS_RESOURCE          sipResource;                                    // Resource to store debug info                 [*]
    MOS_RESOURCE                csrResource;                                    // Resource to store CSR info
    void                        *tableMemories;                                      // Single Memory for all lookup and temp tables [*]
    CM_HAL_HINT_TASK_INDEXES    hintIndexes;                                    // Indexes for EU Saturation API
    bool                        requestSingleSlice;                            // Requests single slice for life of CM device
    bool                        midThreadPreemptionDisabled;                  // set the flag to indicate to disable the midthread preemption
    bool                        kernelDebugEnabled;                            // set the flag to indicate to enable SIP debugging
    PCMLOOKUP_ENTRY             surf2DTable;                                   // Surface registration lookup entries
    PCM_HAL_SURFACE2D_ENTRY     umdSurf2DTable;                                // Surface2D entries used by CMRT@Driver
    PCM_HAL_BUFFER_ENTRY        bufferTable;                                   // Buffer registration table
    PCM_HAL_SURFACE2D_UP_ENTRY  surf2DUPTable;                                 // Buffer registration table
    PCM_HAL_3DRESOURCE_ENTRY    surf3DTable;                                   // 3D surface registration table
    PMHW_SAMPLER_STATE_PARAM    samplerTable;                              // Sampler table
    CM_SAMPLER_STATISTICS       samplerStatistics;
    PCM_HAL_SAMPLER_8X8_ENTRY   sampler8x8Table;                               // Sampler 8x8 table
    char                        *taskStatusTable;                               // Table for task status
    int32_t                     currentTaskEntry;                              // Current task status entry id
    PCM_HAL_MULTI_USE_BTI_ENTRY bti2DIndexTable;                                // Table to store Used 2D binding indexes (temporary)
    PCM_HAL_MULTI_USE_BTI_ENTRY bti2DUPIndexTable;                              // Table to store Used 2D binding indexes (temporary)
    PCM_HAL_MULTI_USE_BTI_ENTRY bti3DIndexTable;                                // Table to store Used 3D binding indexes (temporary)
    PCM_HAL_MULTI_USE_BTI_ENTRY btiBufferIndexTable;                            // Table to store Buffer Binding indexes (temporary)
    char                        *samplerIndexTable;                             // Table to store Used Sampler indexes (temporary)
    char                        *vmeIndexTable;                                 // Table to store Used VME indexes (temporary)
    char                        *sampler8x8IndexTable;                          // Table to store Used Sampler8x8 indexes (temporary)

    CMSURFACE_REG_TABLE         surfaceRegTable;                                // Surface registration table
    CM_HAL_DEVICE_PARAM         cmDeviceParam;                                  // Device Param
    RENDERHAL_KRN_ALLOCATION    kernelParamsRenderHal;                          // RenderHal Kernel Setup
    MHW_KERNEL_PARAM            kernelParamsMhw;                               // MHW Kernel setup
    int32_t                     numBatchBuffers;                               // Number of batch buffers
    uint32_t                    dummyArg;                                     // Dummy Argument for no argument kernel
    CM_HAL_MAX_HW_THREAD_VALUES maxHWThreadValues;                              // Maximum number of hardware threads values
    MHW_VFE_SCOREBOARD          scoreboardParams;                               // Scoreboard Parameters
    MHW_WALKER_PARAMS           walkerParams;                                   // Walker Parameters
    void                        *resourceList;                                  // List of resource handles (temporary) NOTE: Only use this for enqueue

    bool                        nullHwRenderCm;                                // Null rendering flag for Cm function
    HMODULE                     hLibModule;                                     // module handle pointing to the dynamically opened library

    bool                        dshEnabled;                              // Enable Dynamic State Heap
    uint32_t                    dshKernelCacheHit;                            // Kernel Cache hit count
    uint32_t                    dshKernelCacheMiss;                              // Kernel Cache miss count

    RENDERHAL_SURFACE           cmVeboxSurfaces[CM_HAL_MAX_VEBOX_SURF_NUM];     // cm vebox surfaces
    CM_VEBOX_SETTINGS           cmVeboxSettings;                                // cm vebox settings
    RENDERHAL_SURFACE           cmVebeboxParamSurf;
    uint32_t                    cmDebugBTIndex;                                 // cm Debug BT index

    void                        *drmVMap;                                       //for libdrm's patched function "drm_intel_bo_from_vmapping"

    CM_POWER_OPTION             powerOption;                                    // Power option
    bool                        euSaturationEnabled;                           // EU saturation enabled

    int32_t                     kernelNumInGsh;                               // current kernel number in GSH
    int32_t                     *totalKernelSize;                              // Total size table of every kernel in GSH kernel entries

    uint32_t                    surfaceArraySize;                              // size of surface array used for 2D surface alias

    bool                        vtuneProfilerOn;                               // Vtune profiling on or not
    bool                        cbbEnabled;                                    // if conditional batch buffer enabled

    uint32_t                    currentPerfTagIndex[MAX_COMBINE_NUM_IN_PERFTAG];
    std::map<std::string, int>  *perfTagIndexMap[MAX_COMBINE_NUM_IN_PERFTAG];  // mapping from kernel name to perf tag
    MediaPerfProfiler           *perfProfiler;                                 // unified media perf profiler 

    PCM_HAL_GENERIC             cmHalInterface;                                // pointer to genX interfaces

    std::map< void *, CM_HAL_STATE_BUFFER_ENTRY > *state_buffer_list_ptr;      // table of bounded state buffer and kernel ptr

    CmHalL3Settings             l3Settings;

    bool                        useNewSamplerHeap;

    bool                        svmBufferUsed;

    bool                        statelessBufferUsed;

    CMRT_UMD::CSync             *criticalSectionDSH;

    uint32_t                    tsFrequency;

    bool                        forceKernelReload;

    CmExecutionAdv              *advExecutor = nullptr;

    bool                        refactor = false;

    bool                        requestCustomGpuContext = false;

    bool                        veboxDisabled = false;

    bool                        syncOnResource = false;

    // Pointer to the buffer for sychronizing tasks in a queue.
    MOS_RESOURCE                *syncBuffer = nullptr;

    //********************************************************************************
    // Export Interface methods called by CMRT@UMD <START>
    //********************************************************************************
    MOS_STATUS (* pfnCmAllocate)
    (   PCM_HAL_STATE               state);

    MOS_STATUS (* pfnGetMaxValues)
        (   PCM_HAL_STATE           state,
            PCM_HAL_MAX_VALUES      maxValues);

    MOS_STATUS (* pfnGetMaxValuesEx)
        (   PCM_HAL_STATE           state,
            PCM_HAL_MAX_VALUES_EX   maxValuesEx);

    MOS_STATUS (* pfnExecuteTask)
    (   PCM_HAL_STATE               state,
        PCM_HAL_EXEC_TASK_PARAM     param);

    MOS_STATUS (* pfnExecuteGroupTask)
    (   PCM_HAL_STATE                   state,
        PCM_HAL_EXEC_GROUP_TASK_PARAM   param);

    MOS_STATUS (* pfnExecuteVeboxTask)
    (   PCM_HAL_STATE                   state,
        PCM_HAL_EXEC_VEBOX_TASK_PARAM   param);

    MOS_STATUS (* pfnExecuteHintsTask)
    (   PCM_HAL_STATE                   state,
        PCM_HAL_EXEC_HINTS_TASK_PARAM   param);

    MOS_STATUS (* pfnQueryTask)
    (   PCM_HAL_STATE               state,
        PCM_HAL_QUERY_TASK_PARAM    param);

    MOS_STATUS (* pfnRegisterUMDNotifyEventHandle)
    (   PCM_HAL_STATE               state,
        PCM_HAL_OSSYNC_PARAM        param);

    MOS_STATUS (* pfnRegisterSampler)
    (   PCM_HAL_STATE               state,
        PCM_HAL_SAMPLER_PARAM       param);

    MOS_STATUS (* pfnUnRegisterSampler)
    (   PCM_HAL_STATE               state,
        uint32_t                    handle);

    MOS_STATUS (* pfnRegisterSampler8x8)
    (   PCM_HAL_STATE               state,
        PCM_HAL_SAMPLER_8X8_PARAM   param);

    MOS_STATUS (* pfnUnRegisterSampler8x8)
    (   PCM_HAL_STATE               state,
        uint32_t                    handle);

    MOS_STATUS (*pfnAllocateBuffer)
    (   PCM_HAL_STATE               state,
        PCM_HAL_BUFFER_PARAM        param);

    MOS_STATUS (*pfnFreeBuffer)
    (   PCM_HAL_STATE               state,
        uint32_t                    handle);

    MOS_STATUS (*pfnLockBuffer)
    (   PCM_HAL_STATE               state,
        PCM_HAL_BUFFER_PARAM        param);

    MOS_STATUS (*pfnUnlockBuffer)
    (   PCM_HAL_STATE               state,
        PCM_HAL_BUFFER_PARAM        param);

    MOS_STATUS (*pfnAllocateSurface2DUP)
    (   PCM_HAL_STATE               state,
        PCM_HAL_SURFACE2D_UP_PARAM  param);

    MOS_STATUS (*pfnFreeSurface2DUP)
    (   PCM_HAL_STATE               state,
        uint32_t                    handle);

    MOS_STATUS (*pfnGetSurface2DPitchAndSize)
    (   PCM_HAL_STATE               state,
        PCM_HAL_SURFACE2D_UP_PARAM  param);

    MOS_STATUS (*pfnAllocate3DResource)
    (   PCM_HAL_STATE               state,
        PCM_HAL_3DRESOURCE_PARAM    param);

    MOS_STATUS (*pfnFree3DResource)
    (   PCM_HAL_STATE               state,
        uint32_t                    handle);

    MOS_STATUS (*pfnLock3DResource)
    (   PCM_HAL_STATE               state,
        PCM_HAL_3DRESOURCE_PARAM    param);

    MOS_STATUS (*pfnUnlock3DResource)
    (   PCM_HAL_STATE               state,
        PCM_HAL_3DRESOURCE_PARAM    param);

    MOS_STATUS (*pfnAllocateSurface2D)
    (   PCM_HAL_STATE               state,
        PCM_HAL_SURFACE2D_PARAM     param);

    MOS_STATUS (*pfnUpdateSurface2D)
    (   PCM_HAL_STATE                state,
        PCM_HAL_SURFACE2D_PARAM      param);

    MOS_STATUS (*pfnUpdateBuffer)
    (   PCM_HAL_STATE           state,
        PCM_HAL_BUFFER_PARAM    param);

    MOS_STATUS (*pfnFreeSurface2D)
    (   PCM_HAL_STATE               state,
        uint32_t                    handle);

    MOS_STATUS (*pfnLock2DResource)
    (   PCM_HAL_STATE                          state,
        PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM    param);

    MOS_STATUS (*pfnUnlock2DResource)
    (   PCM_HAL_STATE                          state,
        PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM    param);

    MOS_STATUS (*pfnGetSurface2DTileYPitch)
    (   PCM_HAL_STATE               state,
        PCM_HAL_SURFACE2D_PARAM     param);

    MOS_STATUS (*pfnSetCaps)
    (   PCM_HAL_STATE               state,
        PCM_HAL_MAX_SET_CAPS_PARAM  param);

    MOS_STATUS (*pfnGetGPUCurrentFrequency)
    (    PCM_HAL_STATE              state,
        uint32_t                    *gpucurrentFreq);

    MOS_STATUS (*pfnSet2DSurfaceStateParam)
    (   PCM_HAL_STATE                          state,
        PCM_HAL_SURFACE2D_SURFACE_STATE_PARAM  param,
        uint32_t                               aliasIndex,
        uint32_t                               handle);

    MOS_STATUS (*pfnSetBufferSurfaceStatePara) (
     PCM_HAL_STATE                            state,
     PCM_HAL_BUFFER_SURFACE_STATE_PARAM       param);

    MOS_STATUS (*pfnSetSurfaceMOCS) (
     PCM_HAL_STATE                  state,
     uint32_t                       hanlde,
     uint16_t                       mocs,
     uint32_t                       argKind);

    MOS_STATUS (*pfnSetPowerOption)
    (   PCM_HAL_STATE               state,
        PCM_POWER_OPTION            powerOption);

    //********************************************************************************
    // Export Interface methods called by CMRT@UMD <END>
    //********************************************************************************

    //********************************************************************************
    // Internal interface methods called by CM HAL only <START>
    //********************************************************************************
    int32_t (*pfnGetTaskSyncLocation)
    (   PCM_HAL_STATE               state,
        int32_t                     taskId);

    MOS_STATUS (*pfnGetGpuTime)
    (   PCM_HAL_STATE               state,
        uint64_t                    *gpuTime);

    MOS_STATUS (*pfnConvertToQPCTime)
    (   uint64_t                    nanoseconds,
        LARGE_INTEGER               *qpcTime );

    MOS_STATUS (*pfnGetGlobalTime)
    (   LARGE_INTEGER               *globalTime);

    MOS_STATUS (*pfnSendMediaWalkerState)
    (   PCM_HAL_STATE               state,
        PCM_HAL_KERNEL_PARAM        kernelParam,
        PMOS_COMMAND_BUFFER         cmdBuffer);

    MOS_STATUS (*pfnSendGpGpuWalkerState)
    (   PCM_HAL_STATE               state,
        PCM_HAL_KERNEL_PARAM        kernelParam,
        PMOS_COMMAND_BUFFER         cmdBuffer);

    MOS_STATUS (*pfnUpdatePowerOption)
    (   PCM_HAL_STATE               state,
        PCM_POWER_OPTION            powerOption);

    MOS_STATUS (*pfnGetSipBinary)
    (   PCM_HAL_STATE               state);

    MOS_STATUS(*pfnGetPlatformInfo)
    (   PCM_HAL_STATE               state,
        PCM_PLATFORM_INFO           platformInfo,
        bool                        euSaturated);

    MOS_STATUS(*pfnGetGTSystemInfo)
    (   PCM_HAL_STATE               state,
        PCM_GT_SYSTEM_INFO          systemInfo);

    MOS_STATUS (*pfnSetSurfaceReadFlag)
    ( PCM_HAL_STATE           state,
      uint32_t                handle,
      bool                    readSync,
      MOS_GPU_CONTEXT         gpuContext);

    MOS_STATUS (*pfnSetVtuneProfilingFlag)
    (   PCM_HAL_STATE               state,
        bool                        vtuneOn);

    MOS_STATUS(*pfnReferenceCommandBuffer)
    (   PMOS_RESOURCE               osResource,
        void                        **cmdBuffer);

    MOS_STATUS(*pfnSetCommandBufferResource)
    (   PMOS_RESOURCE               osResource,
        void                        **cmdBuffer);

    MOS_STATUS(*pfnEnableTurboBoost)
    (   PCM_HAL_STATE               state);

    MOS_STATUS(*pfnSetCompressionMode)
        (
        PCM_HAL_STATE               state,
        CM_HAL_SURFACE2D_COMPRESSIOM_PARAM  MmcParam
        );

    bool (*pfnIsWASLMinL3Cache)(  );

    MOS_STATUS( *pfnDeleteFromStateBufferList )
        (
        PCM_HAL_STATE               state,
        void                        *kernelPtr );

    PRENDERHAL_MEDIA_STATE( *pfnGetMediaStatePtrForKernel )
        (
        PCM_HAL_STATE               state,
        void                        *kernelPtr );

    uint64_t( *pfnGetStateBufferVAPtrForSurfaceIndex )
        (
        PCM_HAL_STATE               state,
        uint32_t                    surfIndex );

    PRENDERHAL_MEDIA_STATE( *pfnGetMediaStatePtrForSurfaceIndex )
        (
        PCM_HAL_STATE               state,
        uint32_t                    surfIndex );

    uint64_t( *pfnGetStateBufferVAPtrForMediaStatePtr )
        (
        PCM_HAL_STATE               state,
        PRENDERHAL_MEDIA_STATE      mediaStatePtr );

    uint32_t( *pfnGetStateBufferSizeForKernel )
        (
        PCM_HAL_STATE               state,
        void                        *kernelPtr );

    CM_STATE_BUFFER_TYPE( *pfnGetStateBufferTypeForKernel )
        (
        PCM_HAL_STATE               state,
        void                        *kernelPtr );

    MOS_STATUS(*pfnCreateGPUContext)
        (
        PCM_HAL_STATE               state,
        MOS_GPU_CONTEXT             gpuContext,
        MOS_GPU_NODE                gpuNode,
        PMOS_GPUCTX_CREATOPTIONS    mosCreateOption);

    GPU_CONTEXT_HANDLE (*pfnCreateGpuComputeContext) (
        PCM_HAL_STATE state,
        MOS_GPUCTX_CREATOPTIONS *mosCreateOption);

    //*-----------------------------------------------------------------------------
    //| Purpose: Selects the required stream index and sets the correct GPU context for further function calls.
    //| Returns: Previous stream index.
    //| Note: On Linux, context handle is used exclusively to retrieve the correct GPU context. Stream index is used on other operating systems.
    //*-----------------------------------------------------------------------------
    uint32_t (*pfnSetGpuContext)(PCM_HAL_STATE      halState,
                                 MOS_GPU_CONTEXT    contextName,
                                 uint32_t           streamIndex,
                                 GPU_CONTEXT_HANDLE contextHandle);

    MOS_STATUS (*pfnUpdateTrackerResource)
        (
        PCM_HAL_STATE               state,
        PMOS_COMMAND_BUFFER         cmdBuffer,
        uint32_t                    tag);

    //*-----------------------------------------------------------------------------
    //| Purpose: Selects the buffer for synchronizing tasks in a CmQueue.
    //| Returns: Result of this operation.
    //| Note: Synchronization buffer is most useful on Linux. It's not required on other operating systems.
    //*-----------------------------------------------------------------------------
    MOS_STATUS (*pfnSelectSyncBuffer) (PCM_HAL_STATE state, uint32_t bufferIdx);

    //********************************************************************************
    // Internal interface methods called by CM HAL only <END>
    //********************************************************************************

#if (_DEBUG || _RELEASE_INTERNAL)
    bool                        dumpCommandBuffer;                            //flag to enable command buffer dump
    bool                        dumpCurbeData;                                //flag to enable curbe data dump
    bool                        dumpSurfaceContent;                           //flag to enable surface content dump
    bool                        dumpSurfaceState;                             //flag to enable surface state dump
    bool                        enableCMDDumpTimeStamp;                       //flag to enable command buffer dump time stamp
    bool                        enableSurfaceStateDumpTimeStamp;              //flag to enable surface state dump time stamp
    bool                        dumpIDData;                                   //flag to enable surface state dump time stamp
    bool                        enableIDDumpTimeStamp;                        //flag to enable surface state dump time stamp

    int32_t(*pfnInitDumpCommandBuffer)
        (
        PCM_HAL_STATE            state);
    int32_t(*pfnDumpCommadBuffer)
        (
        PCM_HAL_STATE            state,
        PMOS_COMMAND_BUFFER      cmdBuffer,
        int                      offsetSurfaceState,
        size_t                   sizeOfSurfaceState);

    int32_t(*pfnInitDumpSurfaceState)
        (
        PCM_HAL_STATE            state);
    int32_t(*pfnDumpSurfaceState)
        (
        PCM_HAL_STATE            state,
        int                      offsetSurfaceState,
        size_t                   sizeOfSurfaceState);

#endif //(_DEBUG || _RELEASE_INTERNAL)

    MOS_STATUS(*pfnDSHUnregisterKernel)
        (
        PCM_HAL_STATE               state,
        uint64_t                    kernelId);

    uint32_t (*pfnRegisterStream) (PCM_HAL_STATE state);
} CM_HAL_STATE, *PCM_HAL_STATE;

typedef struct _CM_HAL_MI_REG_OFFSETS
{
    uint32_t timeStampOffset;
    uint32_t gprOffset;
} CM_HAL_MI_REG_OFFSETS, *PCM_HAL_MI_REG_OFFSETS;

//*-----------------------------------------------------------------------------
//| HAL CM Index Param
//| Used for temporarily storing indices count used
//*-----------------------------------------------------------------------------
struct CM_HAL_INDEX_PARAM
{
    uint32_t samplerIndexCount;     // [in] sampler indices used
    uint32_t vmeIndexCount;         // [in] VME indices used
    uint32_t sampler8x8IndexCount;  // [in] Sampler8x8 indices used
    uint32_t btArray[8];            // [in] 256 indexes
};
typedef CM_HAL_INDEX_PARAM *PCM_HAL_INDEX_PARAM;

//------------------------------------------------------------------------------
//| Functions
//------------------------------------------------------------------------------

// inline functions
//*-----------------------------------------------------------------------------
//| Purpose: Get a New Task ID for the new task. If not found, return error
//| Returns: Result of the operation
//*-----------------------------------------------------------------------------
__inline MOS_STATUS HalCm_GetNewTaskId(
    PCM_HAL_STATE       state,                                                 // [in]  Pointer to HAL CM State
    int32_t             *piIndex)                                                // [out] Pointer to Task Index
{
    uint32_t i, j;
    uint32_t maxTasks;

    i = state->currentTaskEntry;
    maxTasks = state->cmDeviceParam.maxTasks;

    for (j = maxTasks; j > 0; j--, i = (i + 1) % maxTasks)
    {
        if (state->taskStatusTable[i] == CM_INVALID_INDEX)
        {
            *piIndex = i;
            state->currentTaskEntry = (i + 1) % maxTasks;
            return MOS_STATUS_SUCCESS;
        }
    }

    // Not found
    CM_ASSERTMESSAGE("Unable to find a free slot for Task.");
    return MOS_STATUS_UNKNOWN;
}

MOS_STATUS HalCm_Create(
    PMOS_CONTEXT            osDriverContext,
    PCM_HAL_CREATE_PARAM    cmCreateParam,
    PCM_HAL_STATE           *cmState);

void HalCm_Destroy(
    PCM_HAL_STATE           state);

void HalCm_GetUserFeatureSettings(
    PCM_HAL_STATE           state);

MOS_STATUS HalCm_GetSurfaceDetails(
    PCM_HAL_STATE                   state,
    PCM_HAL_INDEX_PARAM             indexParam,
    uint32_t                        btindex,
    MOS_SURFACE&                    mosSurface,
    int16_t                         globalSurface,
    PRENDERHAL_SURFACE_STATE_ENTRY  surfaceEntry,
    uint32_t                        tempPlaneIndex,
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParam,
    CM_HAL_KERNEL_ARG_KIND          argKind);

MOS_STATUS HalCm_AllocateTsResource(
    PCM_HAL_STATE           state);

MOS_STATUS HalCm_InitializeDynamicStateHeaps(
    PCM_HAL_STATE           state,
    CM_HAL_HEAP_PARAM       *heapParam);

MOS_STATUS HalCm_AllocateTrackerResource(
    PCM_HAL_STATE           state);

MOS_STATUS HalCm_AllocateTables(
    PCM_HAL_STATE           state);

MOS_STATUS HalCm_Allocate(
    PCM_HAL_STATE           state);

MOS_STATUS HalCm_SetupSipSurfaceState(
    PCM_HAL_STATE           state,
    PCM_HAL_INDEX_PARAM     indexParam,
    int32_t                 bindingTable);

//===============<Below are Os-dependent Private/Non-DDI Functions>============================================

void HalCm_OsInitInterface(
    PCM_HAL_STATE           cmState);

MOS_STATUS HalCm_GetSurfaceAndRegister(
    PCM_HAL_STATE           state,
    PRENDERHAL_SURFACE      renderHalSurface,
    CM_HAL_KERNEL_ARG_KIND  surfKind,
    uint32_t                index,
    bool                    pixelPitch);

MOS_STATUS HalCm_SendMediaWalkerState(
    PCM_HAL_STATE           state,
    PCM_HAL_KERNEL_PARAM    kernelParam,
    PMOS_COMMAND_BUFFER     cmdBuffer);

MOS_STATUS HalCm_SendGpGpuWalkerState(
    PCM_HAL_STATE           state,
    PCM_HAL_KERNEL_PARAM    kernelParam,
    PMOS_COMMAND_BUFFER     cmdBuffer);

//===============<Below are Os-non-dependent Private/Non-DDI Functions>=========================================

uint32_t HalCm_GetFreeBindingIndex(
    PCM_HAL_STATE           state,
    PCM_HAL_INDEX_PARAM     indexParam,
    uint32_t                count);

void HalCm_PreSetBindingIndex(
    PCM_HAL_INDEX_PARAM     indexParam,
    uint32_t                start,
    uint32_t                end);

MOS_STATUS HalCm_Setup2DSurfaceStateWithBTIndex(
    PCM_HAL_STATE           state,
    int32_t                 bindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch);

MOS_STATUS HalCm_SetupBufferSurfaceStateWithBTIndex(
    PCM_HAL_STATE           state,
    int32_t                 bindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch);

MOS_STATUS HalCm_Setup2DSurfaceUPStateWithBTIndex(
    PCM_HAL_STATE           state,
    int32_t                 bindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch);

MOS_STATUS HalCm_SetupSampler8x8SurfaceStateWithBTIndex(
    PCM_HAL_STATE           state,
    int32_t                 bindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex,
    bool                    pixelPitch,
    CM_HAL_KERNEL_ARG_KIND  kind,
    uint32_t                addressControl );

MOS_STATUS HalCm_Setup3DSurfaceStateWithBTIndex(
    PCM_HAL_STATE           state,
    int32_t                 bindingTable,
    uint32_t                surfIndex,
    uint32_t                btIndex);

MOS_STATUS HalCm_SyncOnResource(
    PCM_HAL_STATE           state,
    PMOS_SURFACE            surface,
    bool                    isWrite);

void HalCm_OsResource_Unreference(
    PMOS_RESOURCE          osResource);

void HalCm_OsResource_Reference(
    PMOS_RESOURCE            osResource);

MOS_STATUS HalCm_SetSurfaceReadFlag(
    PCM_HAL_STATE           state,
    uint32_t                handle,
    MOS_GPU_CONTEXT         gpuContext);

MOS_STATUS HalCm_SetVtuneProfilingFlag(
    PCM_HAL_STATE           state,
    bool                    vtuneOn);

#if (_DEBUG || _RELEASE_INTERNAL)
int32_t HalCm_InitDumpCommandBuffer(
    PCM_HAL_STATE            state);

int32_t HalCm_DumpCommadBuffer(
    PCM_HAL_STATE            state,
    PMOS_COMMAND_BUFFER      cmdBuffer,
    int                      offsetSurfaceState,
    size_t                   sizeOfSurfaceState);
#endif //(_DEBUG || _RELEASE_INTERNAL)

MOS_STATUS HalCm_Convert_RENDERHAL_SURFACE_To_MHW_VEBOX_SURFACE(
    PRENDERHAL_SURFACE           renderHalSurface,
    PMHW_VEBOX_SURFACE_PARAMS    mhwVeboxSurface);

bool HalCm_IsCbbEnabled(
    PCM_HAL_STATE                           state);

int32_t HalCm_SyncKernel(
    PCM_HAL_STATE                           state,
    uint32_t                                sync);

MOS_STATUS HalCm_GetGfxTextAddress(
    uint32_t                     addressMode,
    MHW_GFX3DSTATE_TEXCOORDMODE  *gfxAddress);

MOS_STATUS HalCm_GetGfxMapFilter(
    uint32_t                     filterMode,
    MHW_GFX3DSTATE_MAPFILTER     *gfxFilter);

MOS_STATUS HalCm_Unlock2DResource(
    PCM_HAL_STATE                           state,
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     param);

MOS_STATUS HalCm_Lock2DResource(
    PCM_HAL_STATE                           state,
    PCM_HAL_SURFACE2D_LOCK_UNLOCK_PARAM     param);

int32_t HalCm_GetTaskSyncLocation(
    PCM_HAL_STATE       state,
    int32_t             taskId);

MOS_STATUS HalCm_SetL3Cache(
    const L3ConfigRegisterValues            *l3Values,
    PCmHalL3Settings                      cmHalL3Cache );

MOS_STATUS HalCm_AllocateSipResource(PCM_HAL_STATE state);

MOS_STATUS HalCm_AllocateCSRResource(PCM_HAL_STATE state);

MOS_STATUS HalCm_OsAddArtifactConditionalPipeControl(
    PCM_HAL_MI_REG_OFFSETS offsets,
    PCM_HAL_STATE state,
    PMOS_COMMAND_BUFFER cmdBuffer,
    int32_t syncOffset,
    PMHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS conditionalParams,
    uint32_t trackerTag);

//!
//! \brief    Get the number of command buffers according to the max task number
//! \details  Get the number of command buffers according to the max task number
//!           the returned number will be passed to pfnCreateGpuContext()
//! \param    [in] osInterface
//!           pointer to OS interface
//! \param    [in] maxTaskNumber
//!           max number of task to support
//! \return   uint32_t
//!
uint32_t HalCm_GetNumCmdBuffers(PMOS_INTERFACE osInterface, uint32_t maxTaskNumber);

void HalCm_GetLegacyRenderHalL3Setting( CmHalL3Settings *l3SettingsPtr, RENDERHAL_L3_CACHE_SETTINGS *l3SettingsLegacyPtr );

MOS_STATUS HalCm_GetNumKernelsPerGroup(
    uint8_t     hintsBits,
    uint32_t    numKernels,
    uint32_t    *numKernelsPerGroup,
    uint32_t    *numKernelGroups,
    uint32_t    *remapKernelToGroup,
    uint32_t    *pRemapGroupToKernel
    );

MOS_STATUS HalCm_GetParallelGraphInfo(
    uint32_t                       maximum,
    uint32_t                       numThreads,
    uint32_t                       width,
    uint32_t                       height,
    PCM_HAL_PARALLELISM_GRAPH_INFO graphInfo,
    CM_DEPENDENCY_PATTERN          pattern,
    bool                           noDependencyCase);

MOS_STATUS HalCm_SetDispatchPattern(
    CM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    CM_DEPENDENCY_PATTERN          pattern,
    uint32_t                       *dispatchFreq
    );

MOS_STATUS HalCm_SetKernelGrpFreqDispatch(
    PCM_HAL_PARALLELISM_GRAPH_INFO  graphInfo,
    PCM_HAL_KERNEL_GROUP_INFO       groupInfo,
    uint32_t                        numKernelGroups,
    uint32_t                        *minSteps);

MOS_STATUS HalCm_SetNoDependKernelDispatchPattern(
    uint32_t                        numThreads,
    uint32_t                        minSteps,
    uint32_t                        *dispatchFreq);

MOS_STATUS HalCm_SetupSamplerState(
    PCM_HAL_STATE                   state,
    PCM_HAL_KERNEL_PARAM            kernelParam,
    PCM_HAL_KERNEL_ARG_PARAM        argParam,
    PCM_HAL_INDEX_PARAM             indexParam,
    int32_t                         mediaID,
    uint32_t                        threadIndex,
    uint8_t                         *buffer);

MOS_STATUS HalCm_SetupBufferSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    int16_t                     globalSurface,
    uint32_t                    threadIndex,
    uint8_t                     *buffer);

MOS_STATUS HalCm_Setup2DSurfaceUPState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer);

MOS_STATUS HalCm_Setup2DSurfaceUPSamplerState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer);

MOS_STATUS HalCm_Setup2DSurfaceSamplerState(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_ARG_PARAM   argParam,
    PCM_HAL_INDEX_PARAM        indexParam,
    int32_t                    bindingTable,
    uint32_t                   threadIndex,
    uint8_t                    *buffer);

MOS_STATUS HalCm_Setup2DSurfaceState(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_ARG_PARAM   argParam,
    PCM_HAL_INDEX_PARAM        indexParam,
    int32_t                    bindingTable,
    uint32_t                   threadIndex,
    uint8_t                    *buffer);

MOS_STATUS HalCm_Setup3DSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer);

MOS_STATUS HalCm_SetupVmeSurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer);

MOS_STATUS HalCm_SetupSampler8x8SurfaceState(
    PCM_HAL_STATE               state,
    PCM_HAL_KERNEL_ARG_PARAM    argParam,
    PCM_HAL_INDEX_PARAM         indexParam,
    int32_t                     bindingTable,
    uint32_t                    threadIndex,
    uint8_t                     *buffer);

uint64_t HalCm_ConvertTicksToNanoSeconds(
    PCM_HAL_STATE               state,
    uint64_t                    ticks);

bool HalCm_IsValidGpuContext(
    MOS_GPU_CONTEXT             gpuContext);

MOS_STATUS HalCm_PrepareVEHintParam(
    PCM_HAL_STATE                  state,
    bool                           bScalable,
    PMOS_VIRTUALENGINE_HINT_PARAMS pVeHintParam);

MOS_STATUS HalCm_DecompressSurface(
    PCM_HAL_STATE              state,
    PCM_HAL_KERNEL_ARG_PARAM   argParam,
    uint32_t                   threadIndex);

MOS_STATUS HalCm_SurfaceSync(
    PCM_HAL_STATE                pState,
    PMOS_SURFACE                 pSurface,
    bool                         bReadSync);

//*-----------------------------------------------------------------------------
//| Helper functions for EnqueueWithHints
//*-----------------------------------------------------------------------------

#endif  // __CM_HAL_H__
