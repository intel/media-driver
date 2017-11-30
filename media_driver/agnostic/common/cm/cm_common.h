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
//! \file      cm_common.h  
//! \brief     Contains CM definitions
//!

#pragma once

#include "mhw_mi.h"
#include "cm_innerdef_os.h"
#include <list>
#if USE_EXTENSION_CODE
#include "cm_private.h"
#endif

//Copied over from auxilarydevice to avoid inclusion of video acceleration APIs.
typedef struct cm_tagLOOKUP_ENTRY
{
    void *osSurfaceHandle;            // Surface defined in video acceleration APIs provided by the OS.
    UMD_RESOURCE umdSurfaceHandle;    // Surface defined in UMD.
    uint32_t            SurfaceAllocationIndex;   // Driver allocation index
} CMLOOKUP_ENTRY, *PCMLOOKUP_ENTRY;

typedef struct cm_tagSURFACE_REG_TABLE
{
    uint32_t         Count;              // Number of entries
    CMLOOKUP_ENTRY   *pEntries;          // Surface Lookup table
} CMSURFACE_REG_TABLE, *PCMSURFACE_REG_TABLE;


// this marco is used to remove warning from clang to unused parameters
// which is harmless and useless but to make clang happy
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif // UNUSED

//======<Feature List>======================================================================

#define MDF_HW_KERNEL_DEBUG_SUPPORT     1 //avaliable by default, will be excluded in Open source media.

#if (_DEBUG || _RELEASE_INTERNAL)
#define MDF_COMMAND_BUFFER_DUMP         1 //avaliable in Debug/Release-internal 
#define MDF_CURBE_DATA_DUMP             1
#define MDF_SURFACE_CONTENT_DUMP        1
#endif

//===============<Definitions>==================================================

// Definition for return code
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
    CM_OPEN_VIDEO_DEVICE_HANDLE_FAILURE         = -17,
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
    CM_INVALID_USER_GPU_CONTEXT_FOR_QUEUE_EX    = -102,


    /*
     * RANGE -10000 ~ -19999 FOR INTERNAL ERROR CODE
     */
    CM_INTERNAL_ERROR_CODE_OFFSET               = -10000,

    /*
     * RANGE <=-20000 AREAD FOR MOST STATUS CONVERSION
     */
    CM_MOS_STATUS_CONVERTED_CODE_OFFSET         = -20000
} CM_RETURN_CODE;


#define VPHAL_CM_MAX_THREADS    "CmMaxThreads"

//------------------------------------------------------------------------------
//| Lock flags
//------------------------------------------------------------------------------
#define CM_HAL_LOCKFLAG_READONLY      0x00000001
#define CM_HAL_LOCKFLAG_WRITEONLY     0x00000002

#define CM_BATCH_BUFFER_REUSE_ENABLE        1
#define CM_MAX_TASKS_DEFAULT                4
#define CM_MAXIMUM_TASKS                    64                                  // used for VTune time collection static arrays
#define CM_MAX_TASKS_EU_SATURATION          4

#define CM_KERNEL_BINARY_BLOCK_SIZE         65536                               // 64 KB
#define CM_KERNEL_BINARY_PADDING_SIZE       128                                 // Padding after kernel binary to WA page fault issue.
#define CM_MAX_KERNELS_PER_TASK             16
#define CM_MAX_SPILL_SIZE_PER_THREAD_IVB    11264                               // 11 KB
#define CM_MAX_SPILL_SIZE_PER_THREAD_HSW_BDW 131072                             // 128 KB
#define CM_MAX_SPILL_SIZE_PER_THREAD_HEVC   16384                               // 16 KB
#define CM_MAX_SPILL_SIZE_PER_THREAD_DEFAULT CM_MAX_SPILL_SIZE_PER_THREAD       //
#define CM_MAX_SAMPLER_TABLE_SIZE           512
#define CM_MAX_SAMPLER_8X8_TABLE_SIZE       2  
#define CM_MAX_AVS_SAMPLER_SIZE             16
#define CM_MAX_3D_SAMPLER_SIZE              16
#define CM_MAX_VME_BINDING_INDEX            3                                   // base + forward + backward
#define CM_MAX_VME_BINDING_INDEX_1          33                                  // base + forward + backward  when >= HSW
#define CM_MAX_BUFFER_SURFACE_TABLE_SIZE    256
#define CM_MAX_2D_SURFACE_UP_TABLE_SIZE     512
#define CM_MAX_2D_SURFACE_TABLE_SIZE        256
#define CM_MAX_3D_SURFACE_TABLE_SIZE        64
#define CM_MAX_USER_THREADS                 262144                              // 512x512 (max TS size) * 64 bytes (sizeof(MO cmd) + 10DW) = 16 MB (max allocated for BB)
#define CM_MAX_USER_THREADS_NO_THREADARG    262144                              // 512x512 (max TS size) * 64 bytes (sizeof(MO cmd) + 10DW) = 16 MB (max allocated for BB)
#define MAX_THREAD_SPACE_WIDTH_PERGROUP     64
#define MAX_THREAD_SPACE_HEIGHT_PERGROUP    64
#define MAX_THREAD_SPACE_DEPTH_PERGROUP     64
#define CM_MAX_BB_SIZE                      16777216                            // 16 MB - Maximum Space allocated for Batch Buffer
#define CM_MAX_ARGS_PER_KERNEL              255                                 // compiler only supports up to 255 arguments
#define CM_MAX_THREAD_PAYLOAD_SIZE          2016                                // 63 GRF
#define CM_MAX_ARG_BYTE_PER_KERNEL          CM_MAX_THREAD_PAYLOAD_SIZE
#define CM_EXTRA_BB_SPACE                   (256 + 8*64)                        // Additional Space in BB for commands other than MEDIA_OBJECT, 8 addtional cachelines to avoid page fault
#define CM_MAX_STATIC_SURFACE_STATES_PER_BT 256                                 // Max possible surface state per binding table 
#define CM_MAX_SURFACE_STATES_PER_BT        64                                  // Pre-set Max Surface state per binding table
#define CM_MAX_SURFACE_STATES               256                                 // Max Surface states
#define CM_PAYLOAD_OFFSET                   32                                  // CM Compiler generates offset by 32 bytes. This need to be subtracted from kernel data offset.
#define CM_PER_KERNEL_ARG_VAL               1                                   // Per Kernel Indication
#define CM_MAX_CURBE_SIZE_PER_TASK          8192                                // 256 GRF
#define CM_MAX_CURBE_SIZE_PER_KERNEL        CM_MAX_THREAD_PAYLOAD_SIZE          // 63 GRF
#define CM_MAX_THREAD_WIDTH_FOR_MW          511
#define CM_MAX_INDIRECT_DATA_SIZE_PER_KERNEL    1984                            // 496 x 4
#define CM_HAL_MAX_DEPENDENCY_COUNT         8
#define CM_HAL_MAX_NUM_2D_ALIASES           10                                  // maximum number of aliases for one 2D surface. Arbitrary - can be increased
#define CM_HAL_MAX_NUM_BUFFER_ALIASES       10                                  // maximum number of aliases for one Buffer. Arbitrary - can be increased


#define CM_MAX_SIP_SIZE                     0x2000                              // 8k system routine size
#define CM_DEBUG_SURFACE_INDEX              252                                 // reserved for tools
#define CM_DEBUG_SURFACE_SIZE               0x10000                             // 64k for all threads
#define CM_CSR_SURFACE_SIZE                 0x800000                            // 8 M Bytes for CSR surface
#define CM_SYNC_QWORD_PER_TASK              2                                   // 2 time stamps are record for a task. 1 for GPU start time stamp, 1 for GPU end time stamp.

#define CM_NULL_SURFACE                     0xFFFF
#define CM_SURFACE_MASK                     0xFFFF
#define CM_DEFAULT_CACHE_TYPE               0xFF00

#define CM_NULL_SURFACE_BINDING_INDEX       0                                   // Reserve 0 for NULL surface
#define CM_MAX_GLOBAL_SURFACE_NUMBER        7

#define CM_RESERVED_SURFACE_NUMBER_FOR_KERNEL_DEBUG        1                    //debug surface
#define CM_GPUWALKER_IMPLICIT_ARG_NUM       6                                   //thread and group spaces  plus thread id in two dimentions.
#define CM_HAL_MAX_VEBOX_SURF_NUM           16

#define __CM_SIP_FILE_PATH_GT1              "c:\\sip\\SIP_BTI_252_GT1.dat"
#define __CM_SIP_FILE_PATH_GT2              "c:\\sip\\SIP_BTI_252_GT2.dat"
#define __CM_SIP_FILE_PATH_GT3              "c:\\sip\\SIP_BTI_252_GT3.dat"
#define __CM_SIP_FILE_PATH_GT4              "c:\\sip\\SIP_BTI_252_GT4.dat"

#define CM_HAL_GPU_CONTEXT_COUNT            2 // MOS_GPU_CONTEXT_RENDER4 and MOS_GPU_CONTEXT_RENDER3

#define CM_INVALID_INDEX                    -1

#define CM_KERNEL_FLAGS_CURBE                       0x00000001
#define CM_KERNEL_FLAGS_NONSTALLING_SCOREBOARD      0x00000002  //bit 1

#define ADDRESS_PAGE_ALIGNMENT_MASK_X64             0xFFFFFFFFFFFFF000ULL
#define ADDRESS_PAGE_ALIGNMENT_MASK_X86             0xFFFFF000

//CM MemObjCtl associated
#define CM_INVALID_MEMOBJCTL            0xFF
#define CM_MEMOBJCTL_CACHE_MASK         0xFF00

//Enqueue with Sync
#define CM_NO_KERNEL_SYNC               0

//Conditional batch buffer end
#define CM_NO_CONDITIONAL_END               0
#define CM_DEFAULT_COMPARISON_VALUE         0
#define CM_MAX_CONDITIONAL_END_CMDS        64


// CM masks used to decode hints for EnqueueWithHints
#define CM_HINTS_MASK_MEDIAOBJECT                  0x1
#define CM_HINTS_MASK_KERNEL_GROUPS                0xE
#define CM_HINTS_NUM_BITS_WALK_OBJ                 0x1
#define CM_HINTS_LEASTBIT_MASK                     1
#define CM_HINTS_DEFAULT_NUM_KERNEL_GRP            1
#define CM_DEFAULT_THREAD_DEPENDENCY_MASK          0xFF
#define CM_REUSE_DEPENDENCY_MASK                   0x1
#define CM_RESET_DEPENDENCY_MASK                   0x2
#define CM_NO_BATCH_BUFFER_REUSE                   0x4
#define CM_NO_BATCH_BUFFER_REUSE_BIT_POS           0x2
#define CM_SCOREBOARD_MASK_POS_IN_MEDIA_OBJECT_CMD 0x5
#define CM_HINTS_MASK_NUM_TASKS                    0x70  
#define CM_HINTS_NUM_BITS_TASK_POS                 0x4


// CM device creation with different options
#define CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_ENABLE        0
#define CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_DISABLE       1
#define CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_MASK          1

#define CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_ENABLE_HEVC   2
#define CM_DEVICE_CREATE_OPTION_MAX_TASKS_HEVC_MASK         4

#define CM_DEVICE_CREATE_OPTION_TDR_DISABLE                 64  //Reserved, used only in CMRT Thin

#define CM_DEVICE_CREATE_OPTION_DEFAULT                     CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_ENABLE

#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_OFFSET          1 
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_MASK            (7 << CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_OFFSET)
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_16K_STEP        16384   //16K
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_16K             1
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_32K             2
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_48K             3
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_64K             4
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_80K             5
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_96K             6
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_112K            7
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_128K            0 // 128K By default
#define CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_DEFAULT         CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_128K

#define CM_DEVICE_CONFIG_TASK_NUM_OFFSET                    4
#define CM_DEVICE_CONFIG_TASK_NUM_MASK                     (3 << CM_DEVICE_CONFIG_TASK_NUM_OFFSET)
#define CM_DEVICE_CONFIG_TASK_NUM_DEFAULT                   0
#define CM_DEVICE_CONFIG_TASK_NUM_8                         1
#define CM_DEVICE_CONFIG_TASK_NUM_12                        2
#define CM_DEVICE_CONFIG_TASK_NUM_16                        3
#define CM_DEVICE_CONFIG_TASK_NUM_STEP                      4

#define CM_DEVICE_CONFIG_MEDIA_RESET_OFFSET                 7
#define CM_DEVICE_CONFIG_MEDIA_RESET_ENABLE                (1 << CM_DEVICE_CONFIG_MEDIA_RESET_OFFSET)

#define CM_DEVICE_CONFIG_EXTRA_TASK_NUM_OFFSET              8
#define CM_DEVICE_CONFIG_EXTRA_TASK_NUM_MASK               (3 << CM_DEVICE_CONFIG_EXTRA_TASK_NUM_OFFSET)
#define CM_DEVICE_CONFIG_EXTRA_TASK_NUM_4                   3

#define CM_DEVICE_CONFIG_SLICESHUTDOWN_OFFSET               10
#define CM_DEVICE_CONFIG_SLICESHUTDOWN_ENABLE              (1 << CM_DEVICE_CONFIG_SLICESHUTDOWN_OFFSET)

#define CM_DEVICE_CONFIG_SURFACE_REUSE_OFFSET               11
#define CM_DEVICE_CONFIG_SURFACE_REUSE_ENABLE               (1 << CM_DEVICE_CONFIG_SURFACE_REUSE_OFFSET)

#define CM_DEVICE_CONFIG_GPUCONTEXT_OFFSET                  12
#define CM_DEVICE_CONFIG_GPUCONTEXT_ENABLE                  (1 << CM_DEVICE_CONFIG_GPUCONTEXT_OFFSET)

#define CM_DEVICE_CONFIG_KERNELBINARYGSH_OFFSET             13
#define CM_DEVICE_CONFIG_KERNELBINARYGSH_MASK               (255 << CM_DEVICE_CONFIG_KERNELBINARYGSH_OFFSET )

#define CM_DEVICE_CONFIG_DSH_DISABLE_OFFSET                 21
#define CM_DEVICE_CONFIG_DSH_DISABLE_MASK                   (1 << CM_DEVICE_CONFIG_DSH_DISABLE_OFFSET)

#define CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET         22
#define CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_DISENABLE         (1 << CM_DEVICE_CONFIG_MIDTHREADPREEMPTION_OFFSET)

#define CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET                23
#define CM_DEVICE_CONFIG_KERNEL_DEBUG_ENABLE               (1 << CM_DEVICE_CONFIG_KERNEL_DEBUG_OFFSET)

// HEVC config : 
// Scratch space size :16k
// Number of task: 16
// Media Reset Option : true
// Extra task num: 4
#define CM_DEVICE_CREATE_OPTION_FOR_HEVC                    ((CM_DEVICE_CREATE_OPTION_SCRATCH_SPACE_ENABLE) \
                                                             | (CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_16K << CM_DEVICE_CONFIG_SCRATCH_SPACE_SIZE_OFFSET) \
                                                             | (CM_DEVICE_CONFIG_TASK_NUM_16 << CM_DEVICE_CONFIG_TASK_NUM_OFFSET) \
                                                             | (CM_DEVICE_CONFIG_MEDIA_RESET_ENABLE) \
                                                             | (CM_DEVICE_CONFIG_EXTRA_TASK_NUM_4 << CM_DEVICE_CONFIG_EXTRA_TASK_NUM_OFFSET)\
                                                             | (CM_DEVICE_CONFIG_GPUCONTEXT_ENABLE))

// HEVC config with slice shutdown enabled
#define CM_DEVICE_CREATE_OPTION_FOR_HEVC_SSD                ((CM_DEVICE_CREATE_OPTION_FOR_HEVC) | (CM_DEVICE_CONFIG_SLICESHUTDOWN_ENABLE))

#define CM_ARGUMENT_SURFACE_SIZE         4

#define MAX_STEPPING_NUM    10

#define CM_26ZI_BLOCK_WIDTH              16
#define CM_26ZI_BLOCK_HEIGHT             8

#define CM_NUM_DWORD_FOR_MW_PARAM        16

#define CM_MAX_KERNEL_NAME_SIZE_IN_BYTE         256
#define CM_EXTRA_STRING_LENGTH_FOR_MWMO_DUMP    50

#define CM_KERNELBINARY_BLOCKSIZE_2MB (1024 * 1024 * 2)
#define CM_64BYTE (64)


#define CM_DDI_1_0 100
#define CM_DDI_1_1 101
#define CM_DDI_1_2 102
#define CM_DDI_1_3 103
#define CM_DDI_1_4 104
#define CM_DDI_2_0 200
#define CM_DDI_2_1 201
#define CM_DDI_2_2 202
#define CM_DDI_2_3 203
#define CM_DDI_2_4 204
#define CM_DDI_3_0 300
#define CM_DDI_4_0 400
#define CM_DDI_5_0 500
#define CM_DDI_6_0 600
#define CM_DDI_7_0 700
#define CM_DDI_7_2 702 //for MDFRT API refreshment.

#define CM_VERSION (CM_DDI_7_2)

//------------------------------------------------------------------------------
//| Forward declarations
//------------------------------------------------------------------------------
typedef struct _CM_HAL_STATE        *PCM_HAL_STATE;
typedef struct _CM_HAL_TASK_PARAM   *PCM_HAL_TASK_PARAM;
typedef struct _CM_HAL_TASK_TIMESTAMP   *PCM_HAL_TASK_TIMESTAMP;
typedef struct _CM_HAL_KERNEL_PARAM  *PCM_HAL_KERNEL_PARAM;
typedef struct _CM_HAL_SAMPLER_8X8_TABLE *PCM_HAL_SAMPLER_8X8_TABLE;
typedef struct _CM_AVS_TABLE_STATE_PARAMS *PCM_AVS_TABLE_STATE_PARAMS;

//------------------------------------------------------------------------------
//| Enumeration for Task Status
//------------------------------------------------------------------------------
typedef enum _CM_HAL_TASK_STATUS
{
    CM_TASK_QUEUED,
    CM_TASK_IN_PROGRESS,
    CM_TASK_FINISHED,
    CM_TASK_RESET
} CM_HAL_TASK_STATUS;

//------------------------------------------------------------------------------
//| CM buffer types
//------------------------------------------------------------------------------
typedef enum _CM_BUFFER_TYPE
{
    CM_BUFFER_N             = 0,
    CM_BUFFER_UP            = 1,  
    CM_BUFFER_SVM           = 2,
    CM_BUFFER_GLOBAL        = 3,
    CM_BUFFER_STATE         = 4
} CM_BUFFER_TYPE;

//------------------------------------------------------------------------------
//| enums for CloneKernel API
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//| CM shift direction. Used for CloneKernel API to adjust head kernel allocation ID.
//| Need to adjust IDs after the kernel allocation entries are shifted
//| i.e. neighboring free kernel allocation entries are combined into a single larger entry
//------------------------------------------------------------------------------
typedef enum _CM_SHIFT_DIRECTION
{
    CM_SHIFT_LEFT  = 0,
    CM_SHIFT_RIGHT = 1
}CM_SHIFT_DIRECTION;

//------------------------------------------------------------------------------
//| CM clone type
//| CM_NO_CLONE: regular kernel, not created from CloneKernel API and has no kernels that were cloned from it
//| CM_CLONE_ENTRY: 64B kernel allocation entry for a cloned kernel (will point to the head kernel's binary)
//| CM_HEAD_KERNEL: kernel allocation entry that contains kernel binary (clone kernels will use this offset)
//| CM_CLONE_AS_HEAD_KERNEL: cloned kernel is serving as a head kernel (original kernel and other clones can use this offset)
//------------------------------------------------------------------------------
typedef enum _CM_CLONE_TYPE
{
    CM_NO_CLONE             = 0,
    CM_CLONE_ENTRY          = 1,
    CM_HEAD_KERNEL          = 2,
    CM_CLONE_AS_HEAD_KERNEL = 3
}CM_CLONE_TYPE;

enum CM_STATE_BUFFER_TYPE
{
    CM_STATE_BUFFER_NONE = 0,
    CM_STATE_BUFFER_CURBE = 1,
};

//------------------------------------------------------------------------------
//| HAL CM Max Values, do not add/delete any entry
//------------------------------------------------------------------------------
typedef struct _CM_HAL_MAX_VALUES
{
    uint32_t    iMaxTasks;                                                      // [in] Max Tasks
    uint32_t    iMaxKernelsPerTask;                                             // [in] Max kernels per task
    uint32_t    iMaxKernelBinarySize;                                           // [in] Max kernel binary size
    uint32_t    iMaxSpillSizePerHwThread;                                       // [in] Max spill size per thread
    uint32_t    iMaxSamplerTableSize;                                           // [in] Max sampler table size
    uint32_t    iMaxBufferTableSize;                                            // [in] Max buffer/bufferUP table Size
    uint32_t    iMax2DSurfaceTableSize;                                         // [in] Max 2D surface table Size
    uint32_t    iMax3DSurfaceTableSize;                                         // [in] Max 3D surface table Size
    uint32_t    iMaxArgsPerKernel;                                              // [in] Max arguments per kernel
    uint32_t    iMaxArgByteSizePerKernel;                                       // [in] Max argument size in byte per kernel
    uint32_t    iMaxSurfacesPerKernel;                                          // [in] Max Surfaces Per Kernel
    uint32_t    iMaxSamplersPerKernel;                                          // [in] Max Samplers per kernel
    uint32_t    iMaxHwThreads;                                                  // [in] Max HW threads
    uint32_t    iMaxUserThreadsPerTask;                                         // [in] Max user threads per task
    uint32_t    iMaxUserThreadsPerTaskNoThreadArg;                              // [in] Max user threads per task with no thread arg
} CM_HAL_MAX_VALUES, *PCM_HAL_MAX_VALUES;

//-------------------------------------------------------------------------------------------------
//| HAL CM Max Values extention which has more entries which are not included in CM_HAL_MAX_VALUES
//-------------------------------------------------------------------------------------------------
typedef struct _CM_HAL_MAX_VALUES_EX
{
    uint32_t    iMax2DUPSurfaceTableSize;                                       // [in] Max 2D UP surface table Size
    uint32_t    iMaxSampler8x8TableSize;                                        // [in] Max sampler 8x8 table size
    uint32_t    iMaxCURBESizePerKernel;                                         // [in] Max CURBE size per kernel
    uint32_t    iMaxCURBESizePerTask;                                           // [in] Max CURBE size per task
    uint32_t    iMaxIndirectDataSizePerKernel;                                  // [in] Max indirect data size per kernel
    uint32_t    iMaxUserThreadsPerMediaWalker;                                  // [in] Max user threads per media walker
    uint32_t    iMaxUserThreadsPerThreadGroup;                                  // [in] Max user threads per thread group
} CM_HAL_MAX_VALUES_EX, *PCM_HAL_MAX_VALUES_EX;

typedef struct _CM_INDIRECT_SURFACE_INFO
{
    uint16_t iKind;                 // Surface kind, values in CM_ARG_KIND. For now, only support ARG_KIND_SURFACE_1D/ARG_KIND_SURFACE_2D/ARG_KIND_SURFACE_2D_UP
    uint16_t iSurfaceIndex;         // Surface handle used in driver
    uint16_t iBindingTableIndex;   // Binding table index
    uint16_t iNumBTIPerSurf;       // Binding table index count for per surface
} CM_INDIRECT_SURFACE_INFO, *PCM_INDIRECT_SURFACE_INFO;

typedef struct _CM_SAMPLER_BTI_ENTRY
{
    uint32_t                     iSamplerIndex;
    uint32_t                     iSamplerBTI;
} CM_SAMPLER_BTI_ENTRY, *PCM_SAMPLER_BTI_ENTRY;

//------------------------------------------------------------------------------
//| HAL CM Create Param 
//------------------------------------------------------------------------------
typedef struct _CM_HAL_CREATE_PARAM
{
    bool        DisableScratchSpace;                                              // Flag to disable Scratch Space
    uint32_t    ScratchSpaceSize;                                                 // Size of Scratch Space per HW thread
    uint32_t    MaxTaskNumber;                                                    // Max Task Number
    bool        bRequestSliceShutdown;                                            // Flag to enable slice shutdown
    bool        EnableSurfaceReuse;                                               // Flag to enable surface reuse
    bool        bRequestCustomGpuContext;                                         // Flag to use CUSTOM GPU Context 
    uint32_t    KernelBinarySizeinGSH;                                            // Size to be reserved in GSH for kernel binary
    bool        bDynamicStateHeap;                                                // Use Dynamic State Heap management
    bool        bDisabledMidThreadPreemption;                                       // Flag to enable mid thread preemption for GPGPU
    bool        bEnabledKernelDebug;                                              //Flag  to enable Kernel debug
} CM_HAL_CREATE_PARAM, *PCM_HAL_CREATE_PARAM;

//------------------------------------------------------------------------------
//| HAL CM Device Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_DEVICE_PARAM
{
    uint32_t    iMaxTasks;                                                      // [in] Max Tasks
    uint32_t    iMaxKernelsPerTask;                                             // [in] Maximum Number of Kernels Per Task
    uint32_t    iMaxKernelBinarySize;                                           // [in] Maximum binary size of the kernel
    uint32_t    iMaxSamplerTableSize;                                           // [in] Max sampler table size
    uint32_t    iMaxBufferTableSize;                                            // [in] Buffer table Size
    uint32_t    iMax2DSurfaceUPTableSize;                                       // [in] 2D surfaceUP table Size
    uint32_t    iMax2DSurfaceTableSize;                                         // [in] 2D surface table Size
    uint32_t    iMax3DSurfaceTableSize;                                         // [in] 3D table Size
    uint32_t    iMaxSampler8x8TableSize;                                        // [in] Max Sampler 8x8 table size
    uint32_t    iMaxPerThreadScratchSpaceSize;                                  // [in] Max per hw thread scratch space size
    uint32_t    iMaxAVSSamplers;                                                // [in] Max Number of AVS Samplers
    int32_t     iMaxGSHKernelEntries;                                           // [in] Max number of kernel entries in GSH
} CM_HAL_DEVICE_PARAM, *PCM_HAL_DEVICE_PARAM;

// Queue option used by multiple context queues
enum CM_QUEUE_TYPE
{
    CM_QUEUE_TYPE_NONE = 0,
    CM_QUEUE_TYPE_RENDER = 1,
    CM_QUEUE_TYPE_COMPUTE = 2,
    CM_QUEUE_TYPE_VEBOX = 3
};

struct CM_QUEUE_CREATE_OPTION
{
    CM_QUEUE_TYPE QueueType : 3;
    bool RunAloneMode       : 1;
    unsigned int Reserved0  : 3;
    bool UserGPUContext     : 1;
    unsigned int GPUContext : 8; // user provided GPU CONTEXT in enum MOS_GPU_CONTEXT, this will override CM_QUEUE_TYPE if set
    unsigned int Reserved2  : 16;
};

const CM_QUEUE_CREATE_OPTION CM_DEFAULT_QUEUE_CREATE_OPTION = { CM_QUEUE_TYPE_RENDER, false, 0, 0, 0, 0 };

//------------------------------------------------------------------------------
//| CM Sampler Param
//------------------------------------------------------------------------------
typedef enum _CM_HAL_PIXEL_TYPE
{
    CM_HAL_PIXEL_UINT,
    CM_HAL_PIXEL_SINT,
    CM_HAL_PIXEL_OTHER
} CM_HAL_PIXEL_TYPE;

typedef struct _CM_HAL_SAMPLER_PARAM
{
    uint32_t        MagFilter;                                      // [in]  Mag Filter
    uint32_t        MinFilter;                                      // [in]  Min Filter
    uint32_t        AddressU;                                       // [in]  Address U
    uint32_t        AddressV;                                       // [in]  Address V
    uint32_t        AddressW;                                       // [in]  Address W
    uint32_t        dwHandle;                                       // [out] Handle 
    
    CM_HAL_PIXEL_TYPE SurfaceFormat;
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
} CM_HAL_SAMPLER_PARAM, *PCM_HAL_SAMPLER_PARAM;


//------------------------------------------------------------------------------
//|GT-PIN
typedef struct _CM_SURFACE_DETAILS
{
    uint32_t            dwWidth;    //width of surface
    uint32_t            dwHeight;   //height of surface, 0 if surface is CmBuffer
    uint32_t            dwDepth;    //depth of surface, 0 if surface is CmBuffer or CmSurface2D

    DdiSurfaceFormat    dwFormat;   //format of surface, UNKNOWN if surface is CmBuffer 
    uint32_t            dwPlaneIndex;//plane Index for this BTI, 0 if surface is not planar surface

    uint32_t            dwPitch;    //pitch of suface, 0 if surface is CmBuffer
    uint32_t            dwSlicePitch; //pitch of a slice in CmSurface3D, 0 if surface is CmBuffer or CmSurface2D
    uint32_t            dwSurfaceBaseAddress;
    uint8_t             u8TiledSurface;//bool
    uint8_t             u8TileWalk;//bool
    uint32_t            dwXOffset;
    uint32_t            dwYOffset;
}CM_SURFACE_DETAILS,*PCM_SURFACE_DETAILS;

typedef struct _CM_HAL_SURFACE_ENTRY_INFO_ARRAY
{
    uint32_t                                            dwMaxEntryNum; 
    uint32_t                                            dwUsedIndex;
    PCM_SURFACE_DETAILS                                 pSurfEntryInfos; 
    uint32_t                                            dwGlobalSurfNum;
    PCM_SURFACE_DETAILS                                 pGlobalSurfInfos;
}CM_HAL_SURFACE_ENTRY_INFO_ARRAY;

typedef struct _CM_HAL_SURFACE_ENTRY_INFO_ARRAYS
{
    uint32_t                            dwKrnNum; 
    CM_HAL_SURFACE_ENTRY_INFO_ARRAY*    pSurfEntryInfosArray; 
}CM_HAL_SURFACE_ENTRY_INFO_ARRAYS;

//| CM scoreboard XY
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SCOREBOARD_XY
{
    int32_t x;
    int32_t y;
} CM_HAL_SCOREBOARD_XY, *PCM_HAL_SCOREBOARD_XY;

//------------------------------------------------------------------------------
//| CM scoreboard XY with mask
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SCOREBOARD_XY_MASK
{
    int32_t x;
    int32_t y;
    uint8_t mask;
    uint8_t resetMask;
} CM_HAL_SCOREBOARD_XY_MASK, *PCM_HAL_SCOREBOARD_XY_MASK;

//------------------------------------------------------------------------------
//| CM scoreboard XY with color,mask and slice-sub-slice select
//------------------------------------------------------------------------------
typedef struct _CM_HAL_SCOREBOARD
{
    int32_t x;
    int32_t y;
    uint8_t mask;
    uint8_t resetMask;
    uint8_t color;
    uint8_t sliceSelect;
    uint8_t subSliceSelect;
} CM_HAL_SCOREBOARD, *PCM_HAL_SCOREBOARD;

//------------------------------------------------------------------------------
//| CM scoreboard XY with mask and resetMask for Enqueue path
//------------------------------------------------------------------------------
typedef struct _CM_HAL_MASK_AND_RESET
{
    uint8_t mask;
    uint8_t resetMask;
}CM_HAL_MASK_AND_RESET, *PCM_HAL_MASK_AND_RESET;

//------------------------------------------------------------------------------
//| CM dependency pattern
//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
//| CM different dispatch patterns for 26ZI media object
//------------------------------------------------------------------------------
typedef enum _CM_26ZI_DISPATCH_PATTERN
{
    VVERTICAL_HVERTICAL_26           = 0,
    VVERTICAL_HHORIZONTAL_26         = 1,
    VVERTICAL26_HHORIZONTAL26        = 2,
    VVERTICAL1X26_HHORIZONTAL1X26    = 3
} CM_26ZI_DISPATCH_PATTERN;

//------------------------------------------------------------------------------
//| CM media walking pattern (used with no dependency)
//------------------------------------------------------------------------------
typedef enum _CM_WALKING_PATTERN
{
    CM_WALK_DEFAULT = 0,
    CM_WALK_WAVEFRONT = 1,
    CM_WALK_WAVEFRONT26 = 2,
    CM_WALK_VERTICAL = 3,
    CM_WALK_HORIZONTAL = 4,
    CM_WALK_WAVEFRONT26X = 5,
    CM_WALK_WAVEFRONT26ZIG = 6,
    CM_WALK_WAVEFRONT45D = 7,
    CM_WALK_WAVEFRONT45XD_2 = 8
} CM_WALKING_PATTERN;

//------------------------------------------------------------------------------
//| CM MEDIA_OBJECT_WALKER GroupID Select
//------------------------------------------------------------------------------
typedef enum _CM_MW_GROUP_SELECT
{
    CM_MW_GROUP_NONE        = 0,
    CM_MW_GROUP_COLORLOOP   = 1,
    CM_MW_GROUP_INNERLOCAL  = 2,
    CM_MW_GROUP_MIDLOCAL    = 3,
    CM_MW_GROUP_OUTERLOCAL  = 4,
    CM_MW_GROUP_INNERGLOBAL = 5,
} CM_MW_GROUP_SELECT;

//------------------------------------------------------------------------------
//| CM BARRIER MODES
//------------------------------------------------------------------------------
typedef enum _CM_BARRIER_MODE
{
    CM_NO_BARRIER           = 0,
    CM_LOCAL_BARRIER        = 1,
    CM_GLOBAL_BARRIER       = 2
} CM_BARRIER_MODE;

//------------------------------------------------------------------------------
//| CM media walking parameters (used for engineering build)
//------------------------------------------------------------------------------
typedef struct _CM_WALKING_PARAMETERS
{
    uint32_t Value[CM_NUM_DWORD_FOR_MW_PARAM];
} CM_WALKING_PARAMETERS, *PCM_WALKING_PARAMETERS;

//------------------------------------------------------------------------------
//| CM dependency information
//------------------------------------------------------------------------------
typedef struct _CM_HAL_DEPENDENCY
{
    uint32_t count;
    int32_t deltaX[CM_HAL_MAX_DEPENDENCY_COUNT];
    int32_t deltaY[CM_HAL_MAX_DEPENDENCY_COUNT];
} CM_HAL_DEPENDENCY;

//------------------------------------------------------------------------------
//| CM batch buffer dirty status
//------------------------------------------------------------------------------
typedef enum _CM_HAL_BB_DIRTY_STATUS
{
    CM_HAL_BB_CLEAN = 0,
    CM_HAL_BB_DIRTY = 1
}CM_HAL_BB_DIRTY_STATUS, *PCM_HAL_BB_DIRTY_STATUS;

//------------------------------------------------------------------------------
//| CM dispatch information for 26Z (for EnqueueWithHints)
//------------------------------------------------------------------------------
typedef struct _CM_HAL_WAVEFRONT26Z_DISPATCH_INFO
{
    uint32_t numWaves;
    uint32_t *pNumThreadsInWave; 
}CM_HAL_WAVEFRONT26Z_DISPATCH_INFO;

//------------------------------------------------------------------------------
//| CM kernel slice and subslice being assigned to (for EnqueueWithHints)
//------------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_SLICE_SUBSLICE
{
    uint32_t slice;
    uint32_t subSlice;
}CM_HAL_KERNEL_SLICE_SUBSLICE, *PCM_HAL_KERNEL_SLICE_SUBSLICE;

//------------------------------------------------------------------------------
//| CM kernel information for EnqueueWithHints to assign subslice
//------------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_SUBLICE_INFO
{
    uint32_t                       numSubSlices;
    uint32_t                       counter;
    PCM_HAL_KERNEL_SLICE_SUBSLICE  pDestination;
}CM_HAL_KERNEL_SUBSLICE_INFO, *PCM_HAL_KERNEL_SUBSLICE_INFO;

//------------------------------------------------------------------------------
//| CM HW platform info
//------------------------------------------------------------------------------
typedef struct _CM_PLATFORM_INFO
{
    uint32_t numSlices;
    uint32_t numSubSlices;
    uint32_t numEUsPerSubSlice;
    uint32_t numHWThreadsPerEU;
    uint32_t numMaxEUsPerPool;
}CM_PLATFORM_INFO, *PCM_PLATFORM_INFO;

//------------------------------------------------------------------------------
//| CM max parallelism information (for EnqueueWithHints)
//------------------------------------------------------------------------------
typedef struct _CM_HAL_PARALLELISM_GRAPH_INFO
{
    uint32_t maxParallelism;
    uint32_t numMaxRepeat;
    uint32_t numSteps;
}CM_HAL_PARALLELISM_GRAPH_INFO, *PCM_HAL_PARALLELISM_GRAPH_INFO;

//------------------------------------------------------------------------------
//| CM kernel group information (for EnqueueWithHints)
//------------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_GROUP_INFO
{
    uint32_t numKernelsFinished;
    uint32_t numKernelsInGroup;
    uint32_t groupFinished;
    uint32_t numStepsInGrp;
    uint32_t freqDispatch;
}CM_HAL_KERNEL_GROUP_INFO, *PCM_HAL_KERNEL_GROUP_INFO;


//------------------------------------------------------------------------------
//| CM max hardware threads
//------------------------------------------------------------------------------
typedef struct _CM_HAL_MAX_HW_THREAD_VALUES
{
    uint32_t userFeatureValue;
    uint32_t APIValue;
} CM_HAL_MAX_HW_THREAD_VALUES;

//------------------------------------------------------------------------------
//| CM conditional batch buffer end information
//------------------------------------------------------------------------------
typedef struct _CM_HAL_CONDITIONAL_BB_END_INFO
{
    uint32_t bufferTableIndex;
    uint32_t offset;
    uint32_t compareValue;
    bool  bDisableCompareMask;
    bool  bEndCurrentLevel;
    uint32_t  operatorCode;
}CM_HAL_CONDITIONAL_BB_END_INFO, *PCM_HAL_CONDITIONAL_BB_END_INFO;

#define  CM_FUSED_EU_DISABLE                 0
#define  CM_FUSED_EU_ENABLE                  1
#define  CM_FUSED_EU_DEFAULT                 CM_FUSED_EU_DISABLE

#define  CM_TURBO_BOOST_DISABLE               0
#define  CM_TURBO_BOOST_ENABLE                1
#define  CM_TURBO_BOOST_DEFAULT              CM_TURBO_BOOST_ENABLE

#if !(USE_EXTENSION_CODE)
typedef struct _CM_TASK_CONFIG
{
    uint32_t turboBoostFlag;         //CM_TURBO_BOOST_DISABLE----disabled, CM_TURBO_BOOST_ENABLE--------enabled.
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;              //reserve 2 uint32_t fields for future extention
}CM_TASK_CONFIG, *PCM_TASK_CONFIG;
#endif

//*-----------------------------------------------------------------------------
//| CM Set Type
//*-----------------------------------------------------------------------------
typedef enum _CM_SET_TYPE
{
    CM_SET_MAX_HW_THREADS,
    CM_SET_HW_L3_CONFIG
} CM_SET_TYPE;


struct L3ConfigRegisterValues
{
    unsigned int config_register0;
    unsigned int config_register1;
    unsigned int config_register2;
    unsigned int config_register3;
};

typedef struct _CM_HAL_MAX_SET_CAPS_PARAM
{
    CM_SET_TYPE Type;
    union
    {
        uint32_t MaxValue;
        L3ConfigRegisterValues L3CacheValues;
    };

} CM_HAL_MAX_SET_CAPS_PARAM, *PCM_HAL_MAX_SET_CAPS_PARAM;

//*-----------------------------------------------------------------------------
//| Execute Group data params
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_EXEC_GROUP_TASK_PARAM
{
    PCM_HAL_KERNEL_PARAM            *pKernels;                          // [in]  Array of Kernel data
    uint32_t                        *piKernelSizes;                      // [in]  Parallel array of Kernel Size
    uint32_t                        iNumKernels;                        // [in]  Number of Kernels in a task
    int32_t                         iTaskIdOut;                         // [out] Task ID 
    uint32_t                        threadSpaceWidth;                   // [in]  thread space width within group
    uint32_t                        threadSpaceHeight;                  // [in]  thread space height within group
    uint32_t                        threadSpaceDepth;                   // [in]  thread space depth within group
    uint32_t                        groupSpaceWidth;                    // [in]  group space width
    uint32_t                        groupSpaceHeight;                   // [in]  group space height
    uint32_t                        groupSpaceDepth;                    // [in]  group space depth
    uint32_t                        iSLMSize;                           // [in]  SLM size per thread group in 1KB unit
    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS   SurEntryInfoArrays;              // [in]  GT-PIN
    void                            *OsData;                             // [out] Used for Linux OS data to pass to event
    uint64_t                        uiSyncBitmap;                       // [in]  synchronization flag among kernels
    bool                            bGlobalSurfaceUsed;                 // [in]  is global surface used 
    uint32_t                        *piKernelCurbeOffset;                // [in]  Array of Kernel Curbe Offset
    uint32_t                        iPreemptionMode;                    // [in] enable preemption
    bool                            bKernelDebugEnabled;                // [in] kernel debug is enabled
    CM_TASK_CONFIG                  taskConfig;                         // [in] task Config
    void                            *user_defined_media_state;           // [in] pointer to a user defined media state heap block
    CM_QUEUE_CREATE_OPTION          queueOption;                        // [in] multiple contexts queue option
} CM_HAL_EXEC_TASK_GROUP_PARAM, *PCM_HAL_EXEC_GROUP_TASK_PARAM;

//*-----------------------------------------------------------------------------
//| Execute Vebox data params
//*-----------------------------------------------------------------------------

typedef struct _CM_VEBOX_STATE
{

    uint32_t    ColorGamutExpansionEnable : 1;
    uint32_t    ColorGamutCompressionEnable : 1;
    uint32_t    GlobalIECPEnable : 1;
    uint32_t    DNEnable : 1;
    uint32_t    DIEnable : 1;
    uint32_t    DNDIFirstFrame : 1;
    uint32_t    DownsampleMethod422to420 : 1;
    uint32_t    DownsampleMethod444to422 : 1;
    uint32_t    DIOutputFrames : 2;
    uint32_t    DemosaicEnable : 1;
    uint32_t    VignetteEnable : 1;
    uint32_t    AlphaPlaneEnable : 1;
    uint32_t    HotPixelFilteringEnable : 1;
    uint32_t    SingleSliceVeboxEnable : 1;
    uint32_t    LaceCorrectionEnable : BITFIELD_BIT(16);
    uint32_t    DisableEncoderStatistics : BITFIELD_BIT(17);
    uint32_t    DisableTemporalDenoiseFilter : BITFIELD_BIT(18);
    uint32_t    SinglePipeEnable : BITFIELD_BIT(19);
    uint32_t    __CODEGEN_UNIQUE(Reserved) : BITFIELD_BIT(20);
    uint32_t    ForwardGammaCorrectionEnable : BITFIELD_BIT(21);
    uint32_t    __CODEGEN_UNIQUE(Reserved) : BITFIELD_RANGE(22, 24);
    uint32_t    StateSurfaceControlBits : BITFIELD_RANGE(25, 31);


}  CM_VEBOX_STATE, *PCM_VEBOX_STATE;




// vebox
typedef struct _CM_VEBOX_SURFACE
{
    uint16_t wSurfaceIndex;
    uint16_t wSurfaceCtrlBits;
} CM_VEBOX_SURFACE, *PCM_VERBOX_SURFACE;

#define VEBOX_SURFACE_NUMBER                 (16)     //MAX

typedef struct _CM_VEBOX_SURFACE_DATA
{
    CM_VEBOX_SURFACE    surfaceEntry[VEBOX_SURFACE_NUMBER];
} CM_VEBOX_SURFACE_DATA, *PCM_VEBOX_SURFACE_DATA;

typedef struct _CM_HAL_EXEC_VEBOX_TASK_PARAM
{
    uint32_t                    uiVeboxVersion;                     // [in] version
    CM_VEBOX_STATE              cmVeboxState;
    void                        *pVeboxParam;                        // CmBuffer, hold the vebox parameters
    uint32_t                    veboxParamIndex;                    // vebox parameter surface index (UMD)
    CM_VEBOX_SURFACE_DATA       CmVeboxSurfaceData;
    int32_t                     iTaskIdOut;                         // [out] Task ID
    void                        *OsData;                             // [out] Used for Linux OS data to pass to event
} CM_HAL_EXEC_VEBOX_TASK_PARAM, *PCM_HAL_EXEC_VEBOX_TASK_PARAM;

typedef struct _CM_HAL_EXEC_HINTS_TASK_PARAM
{
    PCM_HAL_KERNEL_PARAM  *pKernels;                          // [in]  Array of kernel data
    uint32_t              *piKernelSizes;                      // [in]  Parallel array of kernel size
    uint32_t              iNumKernels;                        // [in]  Number of kernels in a task
    int32_t               iTaskIdOut;                         // [out] Task ID
    uint32_t              iHints;                             // [in]  Hints
    uint32_t              iNumTasksGenerated;                 // [in] Number of task generated already for split task
    bool                  isLastTask;                         // [in] Used to split tasks
    void                  *OsData;                             // [out] Used for Linux OS data to pass to event
    uint32_t              *piKernelCurbeOffset;                // [in]  Kernel Curbe offset
    void                  *user_defined_media_state;           // [in]  pointer to a user defined media state heap block
    CM_QUEUE_CREATE_OPTION queueOption;                       // [in] multiple contexts queue option
}CM_HAL_EXEC_HINTS_TASK_PARAM, *PCM_HAL_EXEC_HINTS_TASK_PARAM;

//------------------------------------------------------------------------------
//| HAL CM Query Task Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_QUERY_TASK_PARAM
{
    int32_t                     iTaskId;                                    // [in]  Task ID
    uint32_t                    uiTaskType;                                 // [in]  Task type
    CM_QUEUE_CREATE_OPTION      queueOption;                               // [in]  Queue type
    CM_HAL_TASK_STATUS          status;                                     // [out] Task Status
    uint64_t                    iTaskDuration;                              // [out] Task Duration 
	uint64_t                    iTaskTickDuration;                                     // [out] Task Duration in Ticks
    LARGE_INTEGER               iTaskGlobalCMSubmitTime;                    // [out] The CM task submission time in CPU
    LARGE_INTEGER               iTaskCMSubmitTimeStamp;                     // [out] The CM task submission time in GPU
    LARGE_INTEGER               iTaskHWStartTimeStamp;                      // [out] The task start execution time in GPU
    LARGE_INTEGER               iTaskHWEndTimeStamp;                        // [out] The task end execution time in GPU
} CM_HAL_QUERY_TASK_PARAM, *PCM_HAL_QUERY_TASK_PARAM;

//------------------------------------------------------------------------------
//| HAL CM Register DmaCompleteEvent Handle Param
//------------------------------------------------------------------------------
typedef struct _CM_HAL_OSSYNC_PARAM
{
    HANDLE                      iOSSyncEvent;                          //KMD Notification
} CM_HAL_OSSYNC_PARAM, *PCM_HAL_OSSYNC_PARAM;


//------------------------------------------------------------------------------
//| HAL Sampler 8x8 State Param for Sampler 8x8 Entry in the array
//------------------------------------------------------------------------------
typedef enum _CM_HAL_SAMPLER_8X8_TYPE
{
    CM_SAMPLER8X8_AVS  = 0,                                // AVS sampler8x8 type
    CM_SAMPLER8X8_CONV = 1,                                // CONV sampler8x type
    CM_SAMPLER8X8_MISC = 3,                                // MISC sampler8x8 type
    CM_SAMPLER8X8_NONE
}CM_HAL_SAMPLER_8X8_TYPE;

typedef struct _CM_AVS_COEFF_TABLE{
    float              FilterCoeff_0_0;
    float              FilterCoeff_0_1;
    float              FilterCoeff_0_2;
    float              FilterCoeff_0_3;
    float              FilterCoeff_0_4;
    float              FilterCoeff_0_5;
    float              FilterCoeff_0_6;
    float              FilterCoeff_0_7;
}CM_AVS_COEFF_TABLE;


//*-----------------------------------------------------------------------------
//| Enumeration for Kernel argument type
//*-----------------------------------------------------------------------------
typedef enum _CM_HAL_KERNEL_ARG_KIND
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
} CM_HAL_KERNEL_ARG_KIND;

//*-----------------------------------------------------------------------------
//| HAL CM Kernel Argument Param
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_ARG_PARAM
{
    CM_HAL_KERNEL_ARG_KIND      Kind;                                           // [in] Kind of argument
    uint32_t                    iUnitCount;                                     // [in] 1 if argument is kernel arg, otherwise equal to thread count
    uint32_t                    iUnitSize;                                      // [in] Unit Size of the argument 
    uint32_t                    iPayloadOffset;                                 // [in] Offset to Thread Payload
    bool                        bPerThread;                                     // [in] Per kernel / per thread argument
    uint8_t                     *pFirstValue;                                    // [in] Byte Pointer to First Value.
    uint32_t                    nCustomValue;                                   // [in] CM defined value for the special kind of argument
    uint32_t                    iAliasIndex;                                    // [in] Alias index, used for CmSurface2D alias
    bool                        bAliasCreated;                                  // [in] Whether or not alias was created for this argument
    bool                        bIsNull;                                        // [in] Whether this argument is a null surface
} CM_HAL_KERNEL_ARG_PARAM, *PCM_HAL_KERNEL_ARG_PARAM;

//*-----------------------------------------------------------------------------
//| HAL CM Indirect Surface Param
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_INDIRECT_SURFACE
{
    uint16_t                    iKind;
    uint16_t                    iSurfaceIndex;
    uint16_t                    iBindingTableIndex;
} CM_HAL_INDIRECT_SURFACE, *PCM_HAL_INDIRECT_SURFACE;

//*-----------------------------------------------------------------------------
//| HAL CM Indirect Data Param
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_INDIRECT_DATA_PARAM
{
    uint16_t                    iIndirectDataSize;                              // [in] Indirect Data Size
    uint16_t                    iSurfaceCount;
    uint8_t                     *pIndirectData;                                  // [in] Pointer to Indirect Data Block
    PCM_INDIRECT_SURFACE_INFO   pSurfaceInfo;
} CM_HAL_INDIRECT_DATA_PARAM, *PCM_HAL_INDIRECT_DATA_PARAM;

//*-----------------------------------------------------------------------------
//| HAL CM Sampler BTI Entry
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_SAMPLER_BTI_ENTRY
{
    uint32_t                     iSamplerIndex;
    uint32_t                     iSamplerBTI;
} CM_HAL_SAMPLER_BTI_ENTRY, *PCM_HAL_SAMPLER_BTI_ENTRY;


//*-----------------------------------------------------------------------------
//| HAL CM Sampler BTI Param
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_SAMPLER_BTI_PARAM
{
    CM_HAL_SAMPLER_BTI_ENTRY    SamplerInfo[ CM_MAX_SAMPLER_TABLE_SIZE ];
    uint32_t                     iSamplerCount;
} CM_HAL_SAMPLER_BTI_PARAM, *PCM_HAL_SAMPLER_BTI_PARAM;

typedef struct _CM_HAL_CLONED_KERNEL_PARAM
{
    bool isClonedKernel;
    uint32_t kernelID;
    bool hasClones;
}CM_HAL_CLONED_KERNEL_PARAM;

typedef enum _CM_GPUCOPY_KERNEL_ID
{
    GPU_COPY_KERNEL_UNKNOWN                     = 0x0,

    //cpu -> gpu
    GPU_COPY_KERNEL_GPU2CPU_UNALIGNED_NV12_ID   = 0x1,
    GPU_COPY_KERNEL_GPU2CPU_ALIGNED_NV12_ID     = 0x2,
    GPU_COPY_KERNEL_GPU2CPU_UNALIGNED_ID        = 0x3,
    GPU_COPY_KERNEL_GPU2CPU_ALIGNED_ID          = 0x4,

    //gpu -> cpu
    GPU_COPY_KERNEL_CPU2GPU_NV12_ID             = 0x5,
    GPU_COPY_KERNEL_CPU2GPU_ID                  = 0x6,

    //gpu -> gpu
    GPU_COPY_KERNEL_GPU2GPU_NV12_ID             = 0x7,
    GPU_COPY_KERNEL_GPU2GPU_ID                  = 0x8,

    //cpu -> cpu
    GPU_COPY_KERNEL_CPU2CPU_ID                  = 0x9
} CM_GPUCOPY_KERNEL_ID;

//*-----------------------------------------------------------------------------
//| HAL CM Kernel Threadspace Param
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_THREADSPACE_PARAM
{
    uint16_t                          iThreadSpaceWidth;                        // [in] Kernel Threadspace width
    uint16_t                          iThreadSpaceHeight;                       // [in] Kernel Threadspace height
    CM_DEPENDENCY_PATTERN             patternType;                              // [in] Kernel dependency as enum
    CM_HAL_DEPENDENCY                 dependencyInfo;                           // [in] Kernel dependency
    PCM_HAL_SCOREBOARD                pThreadCoordinates;                       // [in] 
    uint8_t                           reuseBBUpdateMask;                        // [in] 
    CM_HAL_WAVEFRONT26Z_DISPATCH_INFO dispatchInfo;                             // [in]
    uint8_t                           globalDependencyMask;                     // [in] dependency mask in gloabal dependency vectors
    uint8_t                           walkingParamsValid;                       // [in] for engineering build
    CM_WALKING_PARAMETERS             walkingParams;                            // [in] for engineering build
    uint8_t                           dependencyVectorsValid;                   // [in] for engineering build
    CM_HAL_DEPENDENCY                 dependencyVectors;                        // [in] for engineering build
    uint32_t                          colorCountMinusOne;                       // [in] for color count minus one
    CM_MW_GROUP_SELECT                groupSelect;                              // [in] for group select on BDW+
    CM_HAL_BB_DIRTY_STATUS            BBdirtyStatus;                            // [in] batch buffer dirty status
    CM_WALKING_PATTERN                walkingPattern;                           // [in] media walking pattern as enum
} CM_HAL_KERNEL_THREADSPACE_PARAM, *PCM_HAL_KERNEL_THREADSPACE_PARAM;

typedef struct _CM_HAL_WALKER_XY
{
    union
    {
        struct
        {
            uint32_t x   : 16;
            uint32_t y   : 16;
        };
        uint32_t    value;
    };
} CM_HAL_WALKER_XY, *PCM_HAL_WALKER_XY;

// The following enum type must match
// MHW_WALKER_MODE defined in mhw_render.h
typedef enum _CM_HAL_WALKER_MODE
{
    CM_HAL_WALKER_MODE_NOT_SET  = -1,
    CM_HAL_WALKER_MODE_DISABLED = 0,
    CM_HAL_WALKER_MODE_SINGLE   = 1,    // dual = 0, repel = 1
    CM_HAL_WALKER_MODE_DUAL     = 2,    // dual = 1, repel = 0)
    CM_HAL_WALKER_MODE_TRI      = 3,    // applies in BDW GT2 which has 1 slice and 3 sampler/VME per slice
    CM_HAL_WALKER_MODE_QUAD     = 4,    // applies in HSW GT3 which has 2 slices and 2 sampler/VME per slice
    CM_HAL_WALKER_MODE_HEX      = 6,    // applies in BDW GT2 which has 2 slices and 3 sampler/VME per slice
    CM_HAL_WALKER_MODE_OCT      = 8     // may apply in future Gen media architectures
} CM_HAL_WALKER_MODE;

// The following structure must match the structure
// MHW_WALKER_PARAMS defined in mhw_render.h
typedef struct _CM_HAL_WALKER_PARAMS
{
    uint32_t                InterfaceDescriptorOffset   : 5;
    uint32_t                CmWalkerEnable              : 1;
    uint32_t                ColorCountMinusOne          : 8;
    uint32_t                UseScoreboard               : 1;
    uint32_t                ScoreboardMask              : 8;
    uint32_t                MidLoopUnitX                : 2;
    uint32_t                MidLoopUnitY                : 2;
    uint32_t                MiddleLoopExtraSteps        : 5;
    uint32_t                GroupIdLoopSelect           : 24;
    uint32_t                                            : 8;

    uint32_t                InlineDataLength;
    uint8_t                 *pInlineData;
    uint32_t                dwLocalLoopExecCount;
    uint32_t                dwGlobalLoopExecCount;

    CM_HAL_WALKER_MODE      WalkerMode;
    CM_HAL_WALKER_XY        BlockResolution;
    CM_HAL_WALKER_XY        LocalStart;
    CM_HAL_WALKER_XY        LocalEnd;
    CM_HAL_WALKER_XY        LocalOutLoopStride;
    CM_HAL_WALKER_XY        LocalInnerLoopUnit;
    CM_HAL_WALKER_XY        GlobalResolution;
    CM_HAL_WALKER_XY        GlobalStart;
    CM_HAL_WALKER_XY        GlobalOutlerLoopStride;
    CM_HAL_WALKER_XY        GlobalInnerLoopUnit;

    bool                    bAddMediaFlush;
    bool                    bRequestSingleSlice; 
} CM_HAL_WALKER_PARAMS, *PCM_HAL_WALKER_PARAMS;

typedef struct _CM_GPGPU_WALKER_PARAMS
{
    uint32_t            InterfaceDescriptorOffset   : 5;
    uint32_t            CmGpGpuEnable               : 1;
    uint32_t                                        : 26;
    uint32_t            ThreadWidth;
    uint32_t            ThreadHeight;
    uint32_t            ThreadDepth;
    uint32_t            GroupWidth;
    uint32_t            GroupHeight;
    uint32_t            GroupDepth;
} CM_GPGPU_WALKER_PARAMS, *PCM_GPGPU_WALKER_PARAMS;


#define MAX_ELEMENT_TYPE_COUNT 6

typedef struct _CM_SAMPLER_STATISTICS
{
    uint32_t SamplerCount[MAX_ELEMENT_TYPE_COUNT];
    uint32_t SamplerMultiplier[MAX_ELEMENT_TYPE_COUNT];  //used for distinguishing whether need to take two 
    uint32_t Sampler_indexBase[MAX_ELEMENT_TYPE_COUNT];
} CM_SAMPLER_STATISTICS, *_PCM_SAMPLER_STATISTICS;

#define INDEX_ALIGN(index, elemperIndex, base) ((index * elemperIndex)/base + ( (index *elemperIndex % base))? 1:0)

struct SamplerParam
{
    unsigned int sampler_table_index;
    unsigned int heap_offset;
    unsigned int bti;
    unsigned int bti_stepping;
    unsigned int bti_multiplier;
    bool user_defined_bti;
    bool regular_bti;
    unsigned int element_type;
    unsigned int size;
};

//*-----------------------------------------------------------------------------
//| HAL CM Kernel Param
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_KERNEL_PARAM
{
    CM_HAL_KERNEL_ARG_PARAM         CmArgParams[CM_MAX_ARGS_PER_KERNEL];
    CM_SAMPLER_STATISTICS           SamplerStatistics;                              // [in] each sampler element type count in the kernel argument
    uint8_t                         *pKernelData;                                    // [in] Pointer to Kernel data
    uint32_t                        iKernelDataSize;                                // [in] Size of Kernel Data
    uint8_t                         *pMovInsData;                                    // [in] pointer to move instruction data
    uint32_t                        iMovInsDataSize;                                // [in] size of move instructions
    uint8_t                         *pKernelBinary;                                  // [in] Execution code for the kernel
    uint32_t                        iKernelBinarySize;                              // [in] Size of Kernel Binary
    uint32_t                        iNumThreads;                                    // [in] Number of threads
    uint32_t                        iNumArgs;                                       // [in] Number of Kernel Args
    bool                            bPerThreadArgExisted;                           // 
    uint32_t                        iNumSurfaces;                                   // [in] Number of Surfaces used in this kernel
    uint32_t                        iPayloadSize;                                   // [in] Kernel Payload Size
    uint32_t                        iKrnCurbeSize;                                  // [in] total CURBE size, GPGPU 
    uint32_t                        iCurbeSizePerThread;                            // [in] CURBE size per thread
    uint32_t                        iCrsThrdConstDataLn;                            // [in] Cross-thread constant data length HSW+
    uint32_t                        iBarrierMode;                                   // [in] Barrier mode, 0-No barrier, 1-local barrier, 2-global barrier
    uint32_t                        iNumberThreadsInGroup;                          // [in] Number of Threads in Thread Group
    uint32_t                        iSLMSize;                                       // [in] SLM size in 1K-Bytes or 4K-Bytes
    uint32_t                        iSpillSize;                                     // [in] Kernel spill area, obtained from JITTER
    uint32_t                        dwCmFlags;                                      // [in] Kernel flags
    uint64_t                        uiKernelId;                                     // [in] Kernel Id
    CM_HAL_KERNEL_THREADSPACE_PARAM KernelThreadSpaceParam;                         // [in] ThreadSpace Information
    CM_HAL_WALKER_PARAMS            WalkerParams;                                   // [out] Media walker parameters for kernel:filled in HalCm_ParseTask
    bool                            bGlobalSurfaceUsed;                             // [in] Global surface used
    uint32_t                        globalSurface[CM_MAX_GLOBAL_SURFACE_NUMBER];    // [in] Global Surface indexes
    CM_GPGPU_WALKER_PARAMS          GpGpuWalkerParams;                          
    bool                            bKernelDebugEnabled;                            // [in] kernel debug is enabled
    CM_HAL_INDIRECT_DATA_PARAM      IndirectDataParam;
    char                            pKernelName[ CM_MAX_KERNEL_NAME_SIZE_IN_BYTE ]; // [in] A fixed size array to hold the kernel name
    CM_HAL_SAMPLER_BTI_PARAM        SamplerBTIParam; 
    uint32_t                        localId_index;                                  //local ID index has different location with different compiler version
    CM_HAL_CLONED_KERNEL_PARAM      ClonedKernelParam;
    CM_STATE_BUFFER_TYPE            state_buffer_type;
    std::list<SamplerParam>        *sampler_heap;
} CM_HAL_KERNEL_PARAM, *PCM_HAL_KERNEL_PARAM;

typedef enum _CM_HAL_MEMORY_OBJECT_CONTROL_G7
{
    // SNB
    CM_MEMORY_OBJECT_CONTROL_USE_GTT_ENTRY       = 0x0,
    CM_MEMORY_OBJECT_CONTROL_NEITHER_LLC_NOR_MLC = 0x1,
    CM_MEMORY_OBJECT_CONTROL_LLC_NOT_MLC         = 0x2,
    CM_MEMORY_OBJECT_CONTROL_LLC_AND_MLC         = 0x3,

    // IVB
    CM_MEMORY_OBJECT_CONTROL_FROM_GTT_ENTRY      = 0x0,
    CM_MEMORY_OBJECT_CONTROL_L3                  = 0x1,
    CM_MEMORY_OBJECT_CONTROL_LLC                 = 0x2,
    CM_MEMORY_OBJECT_CONTROL_LLC_L3              = 0x3
} CM_HAL_MEMORY_OBJECT_CONTROL_G7;

typedef enum _CM_HAL_MEMORY_OBJECT_CONTROL_G75
{
    CM_MEMORY_OBJECT_CONTROL_USE_PTE                 = 0x0, 
    CM_MEMORY_OBJECT_CONTROL_UC                      = 0x2,
    CM_MEMORY_OBJECT_CONTROL_LLC_ELLC_WB_CACHED      = 0x4,
    CM_MEMORY_OBJECT_CONTROL_ELLC_WB_CACHED          = 0x6,
    CM_MEMORY_OBJECT_CONTROL_L3_USE_PTE              = 0x1,
    CM_MEMORY_OBJECT_CONTROL_L3_UC                   = 0x3,
    CM_MEMORY_OBJECT_CONTROL_L3_LLC_ELLC_WB_CACHED   = 0x5,
    CM_MEMORY_OBJECT_CONTROL_L3_ELLC_WB_CACHED       = 0x7
} CM_HAL_MEMORY_OBJECT_CONTROL_G75;

typedef union _CM_HAL_MEMORY_OBJECT_CONTROL_G8
{
    struct
    {
        uint32_t Age           : 2;
        uint32_t            : 1;
        uint32_t TargetCache   : 2;
        uint32_t CacheControl  : 2;
        uint32_t            : 25;
    } Gen8;

    uint32_t                    DwordValue;
}CM_HAL_MEMORY_OBJECT_CONTROL_G8;

typedef enum _CM_HAL_MEMORY_OBJECT_CONTROL_G9
{
    CM_MEMORY_OBJECT_CONTROL_SKL_DEFAULT     = 0x0,
    CM_MEMORY_OBJECT_CONTROL_SKL_NO_L3       = 0x1,
    CM_MEMORY_OBJECT_CONTROL_SKL_NO_LLC_ELLC = 0x2,
    CM_MEMORY_OBJECT_CONTROL_SKL_NO_LLC      = 0x3,
    CM_MEMORY_OBJECT_CONTROL_SKL_NO_ELLC     = 0x4,
    CM_MEMORY_OBJECT_CONTROL_SKL_NO_LLC_L3   = 0x5,
    CM_MEMORY_OBJECT_CONTROL_SKL_NO_ELLC_L3  = 0x6,
    CM_MEMORY_OBJECT_CONTROL_SKL_NO_CACHE    = 0x7
}CM_HAL_MEMORY_OBJECT_CONTROL_G9;

typedef enum {
  VEBOX_CURRENT_FRAME_INPUT_SURF,           //HSW+
  VEBOX_PREVIOUS_FRAME_INPUT_SURF,          //HSW+
  VEBOX_STMM_INPUT_SURF,                    //HSW+
  VEBOX_STMM_OUTPUT_SURF,                   //HSW+
  VEBOX_DN_CURRENT_FRAME_OUTPUT_SURF,       //HSW+
  VEBOX_CURRENT_FRAME_OUTPUT_SURF,          //HSW+
  VEBOX_PREVIOUS_FRAME_OUTPUT_SURF,         //HSW+
  VEBOX_STATISTICS_OUTPUT_SURF,             //HSW+
  VEBOX_ALPHA_VIGNETTE_CORRECTION_SURF,     //BDW+
  VEBOX_LACE_ACE_RGB_HISTOGRAM_OUTPUT_SURF, //SKL+
  VEBOX_SKIN_SCORE_OUTPUT_SURF              //SKL+
} VEBOX_SURF_USAGE;

typedef enum {
 VME_CURRENT_INDEX          =    0,
 VME_FORWARD_INDEX          =    1,
 VME_BACKWARD_INDEX         =    2,
 VME_SURFACE_INDEX_SIZE     =    3 
}VME_SURFACE_INDEX;

//*-----------------------------------------------------------------------------
//| HAL CM Task Param
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_TASK_PARAM
{
    uint32_t                    uiNumKernels;                                   // [in] number of kernels
    uint64_t                    uiSyncBitmap;                                   // [in] Sync bitmap
    uint32_t                    iBatchBufferSize;                               // [in] Size of Batch Buffer Needed
    uint32_t                    dwVfeCurbeSize;                                 // [out] Sum of CURBE Size
    uint32_t                    dwUrbEntrySize;                                 // [out] Maximum Payload Size
    CM_HAL_SCOREBOARD           **ppThreadCoordinates;                          // [in] Scoreboard(x,y)
    CM_DEPENDENCY_PATTERN       DependencyPattern;                              // [in] pattern
    uint32_t                    threadSpaceWidth;                               // [in] width
    uint32_t                    threadSpaceHeight;                              // [in] height
    uint32_t                    groupSpaceWidth;                                // [in] group space width
    uint32_t                    groupSpaceHeight;                               // [in] group space height
    uint32_t                    SLMSize;                                        // [in] size of SLM
    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS   SurEntryInfoArrays;                      // [in] GTPin
    uint32_t                    iCurKrnIndex;
    uint32_t                    ColorCountMinusOne;                             // [in] color count
    PCM_HAL_MASK_AND_RESET      *ppDependencyMasks;                             // [in]  Thread dependency masks
    uint8_t                     reuseBBUpdateMask;                              // [in] re-use batch buffer and just update mask
    uint32_t                    surfacePerBT;                                   // [out] surface number for binding table
    bool                        blGpGpuWalkerEnabled;                           // [out]
    CM_WALKING_PATTERN          WalkingPattern;                                 // [in] media walking pattern
    uint32_t                    iPreemptionMode;                                //[in] enable preemption
    bool                        HasBarrier;                                     //[in] enable preemption
    uint8_t                     walkingParamsValid;                             // [in] for engineering build
    CM_WALKING_PARAMETERS       walkingParams;                                  // [in] for engineering build
    uint8_t                     dependencyVectorsValid;                         // [in] for engineering build
    CM_HAL_DEPENDENCY           dependencyVectors;                              // [in] for engineering build
    CM_MW_GROUP_SELECT          MediaWalkerGroupSelect;                         // [in] 
    uint32_t                    KernelDebugEnabled;                             // [in]
    uint64_t                    uiConditionalEndBitmap;                         // [in] conditional end bitmap
    CM_HAL_CONDITIONAL_BB_END_INFO conditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS]; // [in] conditional BB end info used to fill conditionalBBEndParams
    MHW_MI_CONDITIONAL_BATCH_BUFFER_END_PARAMS conditionalBBEndParams[CM_MAX_CONDITIONAL_END_CMDS];
    CM_TASK_CONFIG              taskConfig;                                     // [in] task Config
    void                       *user_defined_media_state;                       // [in] pointer to a user defined media state heap block
    unsigned int                sampler_offsets_by_kernel[CM_MAX_KERNELS_PER_TASK]; // [in] each kernel's sampler heap offset from the DSH sampler heap base
    unsigned int                sampler_counts_by_kernel[CM_MAX_KERNELS_PER_TASK];  // [in] each kernel's sampler count
    unsigned int                sampler_indirect_offsets_by_kernel[CM_MAX_KERNELS_PER_TASK];    // [in] each kernel's indirect sampler heap offset from the DSH sampler heap base
} CM_HAL_TASK_PARAM;

typedef struct _CM_HAL_TASK_TIMESTAMP
{
    LARGE_INTEGER               iGlobalCmSubmitTime[CM_MAXIMUM_TASKS];              // [out] The CM task submission time in CPU
    uint64_t                    iCMSubmitTimeStamp[CM_MAXIMUM_TASKS];               // [out] The CM task submission time in GPU
}CM_HAL_TASK_TIMESTAMP;

typedef struct _CM_HAL_HINT_TASK_INDEXES
{
    uint32_t                    iKernelIndexes[CM_MAX_TASKS_EU_SATURATION];      // [in/out] kernel indexes used for EU saturation
    uint32_t                    iDispatchIndexes[CM_MAX_TASKS_EU_SATURATION];    // [in/out] dispatch indexes used for EU saturation
}CM_HAL_HINT_TASK_INDEXES;

//*-----------------------------------------------------------------------------
//| HAL CM Index Param
//| Used for temporarily storing indices count used
//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_INDEX_PARAM
{
    uint32_t                    dwSamplerIndexCount;                            // [in] sampler indices used
    uint32_t                    dwVmeIndexCount;                                // [in] VME indices used
    uint32_t                    dwSampler8x8IndexCount;                         // [in] Sampler8x8 indices used
    uint32_t                    dwBTArray[8];                                   // [in] 256 indexes
} CM_HAL_INDEX_PARAM, *PCM_HAL_INDEX_PARAM;

//*-----------------------------------------------------------------------------
typedef struct _CM_HAL_EXEC_TASK_PARAM
{
    PCM_HAL_KERNEL_PARAM               *pKernels;              // [in]  Array of Kernel data
    uint32_t                           *piKernelSizes;          // [in]  Parallel array of Kernel Size
    uint32_t                           iNumKernels;            // [in]  Number of Kernels in a task
    int32_t                            iTaskIdOut;             // [out] Task ID 
    CM_HAL_SCOREBOARD                **ppThreadCoordinates;    // [in]  Scoreboard(x,y)
    CM_DEPENDENCY_PATTERN              DependencyPattern;      // [in]  pattern
    uint32_t                           threadSpaceWidth;       // [in]  width
    uint32_t                           threadSpaceHeight;      // [in]  height
    CM_HAL_SURFACE_ENTRY_INFO_ARRAYS   SurEntryInfoArrays;     // [out] Used by GT-Pin
    void                               *OsData;                 // [out] Used for Linux OS data to pass to event
    uint32_t                           ColorCountMinusOne;     // [in]
    PCM_HAL_MASK_AND_RESET            *ppDependencyMasks;      // [in]  media object thread dependency masks
    uint64_t                           uiSyncBitmap;           // [in] bit map for sync b/w kernels
    bool                               bGlobalSurfaceUsed;     // [in] if global surface used
    uint32_t                           *piKernelCurbeOffset;    // [in]  array of kernel's curbe offset
    CM_WALKING_PATTERN                 WalkingPattern;         // [in]  media walking pattern
    uint8_t                            walkingParamsValid;     // [in] for engineering build
    CM_WALKING_PARAMETERS              walkingParams;          // [in] for engineering build
    uint8_t                            dependencyVectorsValid; // [in] for engineering build
    CM_HAL_DEPENDENCY                  dependencyVectors;      // [in] for engineering build
    CM_MW_GROUP_SELECT                 MediaWalkerGroupSelect; // [in] 
    bool                               bKernelDebugEnabled;    // [in] kernel debug is enabled
    uint64_t                           uiConditionalEndBitmap; // [in] bit map for conditional end b/w kernels
    CM_HAL_CONDITIONAL_BB_END_INFO     ConditionalEndInfo[CM_MAX_CONDITIONAL_END_CMDS];
    CM_TASK_CONFIG                     taskConfig;             // [in] task Config
    void                              *user_defined_media_state; // [in] pointer to a user defined media state heap block
    CM_QUEUE_CREATE_OPTION             queueOption;           // [in] multiple contexts queue option
} CM_HAL_EXEC_TASK_PARAM, *PCM_HAL_EXEC_TASK_PARAM;

typedef struct _CM_POWER_OPTION
{
    uint16_t nSlice;                      // set number of slice to use: 0(default number), 1, 2...
    uint16_t nSubSlice;                   // set number of subslice to use: 0(default number), 1, 2... 
    uint16_t nEU;                         // set number of EU to use: 0(default number), 1, 2...
} CM_POWER_OPTION, *PCM_POWER_OPTION;

typedef enum {
    UN_PREEMPTABLE_MODE,
    COMMAND_BUFFER_MODE,
    THREAD_GROUP_MODE,
    MIDDLE_THREAD_MODE
} CM_PREEMPTION_MODE;

typedef enum _CM_EVENT_PROFILING_INFO
{
    CM_EVENT_PROFILING_HWSTART,
    CM_EVENT_PROFILING_HWEND,
    CM_EVENT_PROFILING_SUBMIT,
    CM_EVENT_PROFILING_COMPLETE,
    CM_EVENT_PROFILING_ENQUEUE,
    CM_EVENT_PROFILING_KERNELCOUNT,
    CM_EVENT_PROFILING_KERNELNAMES,
    CM_EVENT_PROFILING_THREADSPACE,
    CM_EVENT_PROFILING_CALLBACK
}CM_EVENT_PROFILING_INFO;


//*-----------------------------------------------------------------------------
//| CM Query Type
//*-----------------------------------------------------------------------------
typedef enum _CM_QUERY_TYPE
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
} CM_QUERY_TYPE;

//*-----------------------------------------------------------------------------
//| CM Query Caps
//*-----------------------------------------------------------------------------
typedef struct _CM_QUERY_CAPS
{
    CM_QUERY_TYPE Type;
    union
    {
        int32_t                 iVersion;
        HANDLE                  hRegistration;
        CM_HAL_MAX_VALUES       MaxValues;
        CM_HAL_MAX_VALUES_EX    MaxValuesEx;
        uint32_t                MaxVmeTableSize;
        uint32_t                genCore;
        uint32_t                genGT;
        uint32_t                MinRenderFreq;
        uint32_t                MaxRenderFreq;
        uint32_t                genStepId;
        uint32_t                GPUCurrentFreq;
        uint32_t                Surface2DCount;
        GMM_RESOURCE_FORMAT     *pSurface2DFormats;
        CM_PLATFORM_INFO        PlatformInfo;
    };
} CM_QUERY_CAPS, *PCM_QUERY_CAPS;


//*-----------------------------------------------------------------------------
//| CM Task Profiling Information
//*-----------------------------------------------------------------------------
typedef struct _CM_PROFILING_INFO
{
    uint32_t                dwTaskID;
    uint32_t                dwThreadID;
    uint32_t                dwKernelCount;
    uint32_t                dwKernelNameLen;
    char                    *pKernelNames;
    uint32_t                *pLocalWorkWidth;
    uint32_t                *pLocalWorkHeight;
    uint32_t                *pGlobalWorkWidth;
    uint32_t                *pGlobalWorkHeight;
    LARGE_INTEGER           EnqueueTime;
    LARGE_INTEGER           FlushTime;
    LARGE_INTEGER           HwStartTime;
    LARGE_INTEGER           HwEndTime;
    LARGE_INTEGER           CompleteTime;
} CM_PROFILING_INFO, *PCM_PROFILING_INFO;

typedef struct _CM_SURFACE_BTI_INFO
{
    uint32_t dwNormalSurfaceStart;   // start index of normal surface 
    uint32_t dwNormalSurfaceEnd;     // end index of normal surface
    uint32_t dwReservedSurfaceStart; // start index of reserved surface
    uint32_t dwReservedSurfaceEnd;   // end index of reserved surface
}CM_SURFACE_BTI_INFO, *PCM_SURFACE_BTI_INFO;

//*-----------------------------------------------------------------------------
//| CM Convolve type for SKL+
//*-----------------------------------------------------------------------------
typedef enum _CM_CONVOLVE_SKL_TYPE
{
    CM_CONVOLVE_SKL_TYPE_2D = 0,
    CM_CONVOLVE_SKL_TYPE_1D = 1,
    CM_CONVOLVE_SKL_TYPE_1P = 2
} CM_CONVOLVE_SKL_TYPE;

//*-----------------------------------------------------------------------------
//| Compression state 
//*-----------------------------------------------------------------------------

typedef enum _MEMCOMP_STATE
{
    MEMCOMP_DISABLED = 0,
    MEMCOMP_HORIZONTAL,
    MEMCOMP_VERTICAL
}MEMCOMP_STATE, *PMEMCOMP_STATE;

// to define frame type for interlace frame support
typedef enum _CM_FRAME_TYPE
{
    CM_FRAME,     // singe frame, not interlaced
    CM_TOP_FIELD,
    CM_BOTTOM_FIELD,
    MAX_FRAME_TYPE
} CM_FRAME_TYPE;

//L3 Configurations
typedef struct _L3_CONFIG {
    uint32_t    slm;            //shared local memory
    uint32_t    urb;            //unified return buffer 
    uint32_t    rest;           //rest 
    uint32_t    datacluster;    //data cluster 
    uint32_t    readonly;       //read only
    uint32_t    instruction;    //instruction/state cache
    uint32_t    constant;       //constant cache
    uint32_t    texture;        //texture cache
    uint32_t    sum;            //sum
} L3_CONFIG;

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

