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

#ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMCOMMON_H_
#define MEDIADRIVER_AGNOSTIC_COMMON_CM_CMCOMMON_H_

#include "mhw_mi.h"
#include "cm_innerdef_os.h"
#include <list>

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
#define MDF_SURFACE_STATE_DUMP          1
#define MDF_INTERFACE_DESCRIPTOR_DATA_DUMP  1
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
    CM_INVALID_CREATE_OPTION_FOR_BUFFER_STATELESS = -103,
    CM_INVALID_KERNEL_ARG_POINTER                 = -104,
    CM_SYSTEM_MEMORY_NOT_2PIXELS_ALIGNED          = -105,

    /*
     * RANGE -10000 ~ -19999 FOR INTERNAL ERROR CODE
     */
    CM_INTERNAL_ERROR_CODE_OFFSET               = -10000,

    /*
     * RANGE <=-20000 AREAD FOR MOST STATUS CONVERSION
     */
    CM_MOS_STATUS_CONVERTED_CODE_OFFSET         = -20000
} CM_RETURN_CODE;

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

#define CM_MAX_SIP_SIZE                     0x4000                              // 16k system routine size
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

#define CM_DEVICE_CONFIG_VEBOX_OFFSET                       28
#define CM_DEVICE_CONFIG_VEBOX_DISABLE                      (1 << CM_DEVICE_CONFIG_VEBOX_OFFSET)

#define CM_DEVICE_CONFIG_GPUCOPY_OFFSET                     29
#define CM_DEVICE_CONFIG_GPUCOPY_DISABLE                    (1 << CM_DEVICE_CONFIG_GPUCOPY_OFFSET)

#define CM_DEVICE_CONFIG_FAST_PATH_OFFSET                   30
#define CM_DEVICE_CONFIG_FAST_PATH_ENABLE                   (1 << CM_DEVICE_CONFIG_FAST_PATH_OFFSET)

#define CM_DEVICE_CONFIG_MOCK_RUNTIME_OFFSET                31
#define CM_DEVICE_CONFIG_MOCK_RUNTIME_ENABLE                (1 << CM_DEVICE_CONFIG_MOCK_RUNTIME_OFFSET)

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

#define CM_BUFFER_STATELESS_CREATE_OPTION_GFX_MEM 0
#define CM_BUFFER_STATELESS_CREATE_OPTION_SYS_MEM 1
#define CM_BUFFER_STATELESS_CREATE_OPTION_DEFAULT CM_BUFFER_STATELESS_CREATE_OPTION_GFX_MEM

//------------------------------------------------------------------------------
//| Forward declarations
//------------------------------------------------------------------------------
typedef struct _CM_HAL_STATE        *PCM_HAL_STATE;

struct CM_HAL_KERNEL_PARAM;
typedef CM_HAL_KERNEL_PARAM  *PCM_HAL_KERNEL_PARAM;

// Queue option used by multiple context queues
enum CM_QUEUE_TYPE
{
    CM_QUEUE_TYPE_NONE = 0,
    CM_QUEUE_TYPE_RENDER = 1,
    CM_QUEUE_TYPE_COMPUTE = 2
};

enum CM_QUEUE_SSEU_USAGE_HINT_TYPE
{
    CM_QUEUE_SSEU_USAGE_HINT_DEFAULT = 0,
    CM_QUEUE_SSEU_USAGE_HINT_VME  = 1
};

struct _CM_QUEUE_CREATE_OPTION
{
    CM_QUEUE_TYPE                 QueueType               : 3;
    bool                          RAMode                  : 1;
    unsigned int                  Reserved0               : 3;
    bool                          UserGPUContext          : 1; // Is the user-provided GPU Context already created externally
    unsigned int                  GPUContext              : 8; // user-provided GPU Context ordinal
    CM_QUEUE_SSEU_USAGE_HINT_TYPE SseuUsageHint           : 3;
    unsigned int                  Reserved1               : 1;
    unsigned int                  Reserved2               : 12;
};
#define CM_QUEUE_CREATE_OPTION _CM_QUEUE_CREATE_OPTION

const CM_QUEUE_CREATE_OPTION CM_DEFAULT_QUEUE_CREATE_OPTION = { CM_QUEUE_TYPE_RENDER, false, 0, false, 0, CM_QUEUE_SSEU_USAGE_HINT_DEFAULT, 0, 0 };

//------------------------------------------------------------------------------
//|GT-PIN
typedef struct _CM_SURFACE_DETAILS
{
    uint32_t            width;                  //width of surface
    uint32_t            height;                 //height of surface, 0 if surface is CmBuffer
    uint32_t            depth;                  //depth of surface, 0 if surface is CmBuffer or CmSurface2D

    DdiSurfaceFormat    format;                 //format of surface, UNKNOWN if surface is CmBuffer
    uint32_t            planeIndex;             //plane Index for this BTI, 0 if surface is not planar surface

    uint32_t            pitch;                  //pitch of suface, 0 if surface is CmBuffer
    uint32_t            slicePitch;             //pitch of a slice in CmSurface3D, 0 if surface is CmBuffer or CmSurface2D
    uint32_t            surfaceBaseAddress;
    uint8_t             tiledSurface;           //bool
    uint8_t             tileWalk;               //bool
    uint32_t            xOffset;
    uint32_t            yOffset;
}CM_SURFACE_DETAILS,*PCM_SURFACE_DETAILS;

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
    CM_WALK_WAVEFRONT45XD_2 = 8,
    CM_WALK_WAVEFRONT26XALT = 9,
    CM_WALK_WAVEFRONT26D = 10,
    CM_WALK_WAVEFRONT26XD = 11
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
//| CM media walking parameters (used for engineering build)
//------------------------------------------------------------------------------
typedef struct _CM_WALKING_PARAMETERS
{
    uint32_t Value[CM_NUM_DWORD_FOR_MW_PARAM];
} CM_WALKING_PARAMETERS, *PCM_WALKING_PARAMETERS;

#define  CM_FUSED_EU_DISABLE                 0
#define  CM_FUSED_EU_ENABLE                  1
#define  CM_FUSED_EU_DEFAULT                 CM_FUSED_EU_DISABLE

#define  CM_TURBO_BOOST_DISABLE               0
#define  CM_TURBO_BOOST_ENABLE                1
#define  CM_TURBO_BOOST_DEFAULT              CM_TURBO_BOOST_ENABLE

typedef struct _CM_TASK_CONFIG
{
    bool     turboBoostFlag      : 1;
    bool     fusedEuDispatchFlag : 1;
    uint32_t reserved_bits       :30;
    uint32_t reserved0;
    uint32_t reserved1;
    uint32_t reserved2;
}CM_TASK_CONFIG, *PCM_TASK_CONFIG;

typedef enum _CM_KERNEL_EXEC_MODE
{
    CM_KERNEL_EXECUTION_MODE_MONOPOLIZED =  0, // Kernel need occupy all DSS for execution.
    CM_KERNEL_EXECUTION_MODE_CONCURRENT,       // Kernel can occupy part of DSS and concurrently execute together with other workloads.
} CM_KERNEL_EXEC_MODE;

struct CM_EXECUTION_CONFIG
{
    CM_KERNEL_EXEC_MODE kernelExecutionMode = CM_KERNEL_EXECUTION_MODE_MONOPOLIZED;
    int                 concurrentPolicy    = 0; //Reserve for future extension.
};

struct CM_KERNEL_SYNC_CONFIG {
    bool     dataCacheFlush   : 1; // true: cache will be flushed;
    uint32_t reserved         : 31;
};

struct L3ConfigRegisterValues
{
    unsigned int config_register0;
    unsigned int config_register1;
    unsigned int config_register2;
    unsigned int config_register3;
};

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

// referenced in both g9 and g10.
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

// Unified  CM_MEMORY_OBJECT_CONTROL enumeration
typedef enum _CM_HAL_MEMORY_OBJECT_CONTROL
{
    CM_MEMORY_OBJECT_CONTROL_DEFAULT          = 0x0,
    CM_MEMORY_OBJECT_CONTROL_NO_L3            = 0x1,
    CM_MEMORY_OBJECT_CONTROL_NO_LLC_ELLC      = 0x2,
    CM_MEMORY_OBJECT_CONTROL_NO_LLC           = 0x3,
    CM_MEMORY_OBJECT_CONTROL_NO_ELLC          = 0x4,
    CM_MEMORY_OBJECT_CONTROL_NO_LLC_L3        = 0x5,
    CM_MEMORY_OBJECT_CONTROL_NO_ELLC_L3       = 0x6,
    CM_MEMORY_OBJECT_CONTROL_NO_CACHE         = 0x7,
    CM_MEMORY_OBJECT_CONTROL_L1_ENABLED       = 0x8
}CM_HAL_MEMORY_OBJECT_CONTROL;


typedef struct _CM_POWER_OPTION
{
    uint16_t nSlice;                      // set number of slice to use: 0(default number), 1, 2...
    uint16_t nSubSlice;                   // set number of subslice to use: 0(default number), 1, 2...
    uint16_t nEU;                         // set number of EU to use: 0(default number), 1, 2...
} CM_POWER_OPTION, *PCM_POWER_OPTION;

//*-----------------------------------------------------------------------------
//| CM Convolve type for SKL+
//*-----------------------------------------------------------------------------
typedef enum _CM_CONVOLVE_SKL_TYPE
{
    CM_CONVOLVE_SKL_TYPE_2D = 0,
    CM_CONVOLVE_SKL_TYPE_1D = 1,
    CM_CONVOLVE_SKL_TYPE_1P = 2
} CM_CONVOLVE_SKL_TYPE;

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
    uint32_t    slm;            //sharedlocalmemory
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

enum SURFACE_DESTROY_KIND{
    APP_DESTROY         = 0,
    DELAYED_DESTROY     = 1,
    FORCE_DESTROY       = 2
};

// Need to consistant with compiler
enum CM_ARG_KIND
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
    ARG_KIND_SURFACE_2D_SCOREBOARD = 0x2A,  //used for SW scoreboarding
    ARG_KIND_GENERAL_DEPCNT = 0x30          //dependency count, used for SW scoreboarding
};

//non-depend on rtti::dynamic_cast
enum CM_ENUM_CLASS_TYPE
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
};

#endif  // #ifndef MEDIADRIVER_AGNOSTIC_COMMON_CM_CMCOMMON_H_
