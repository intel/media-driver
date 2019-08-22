/*
* Copyright (c) 2009-2019, Intel Corporation
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
//! \file      renderhal.h 
//! \brief 
//!
//!
//! \file     renderhal.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __RENDERHAL_H__
#define __RENDERHAL_H__

#include "mos_os.h"                     // Interface to OS functions
#include "mhw_state_heap.h"
#include "mhw_render.h"

#include "renderhal_dsh.h"
#include "mhw_memory_pool.h"
#include "cm_hal_hashtable.h"
#include "media_perf_profiler.h"

#include "frame_tracker.h"


class XRenderHal_Platform_Interface;

//------------------------------------------------------------------------------
// Macros specific to RenderHal sub-comp
//------------------------------------------------------------------------------
#define MHW_RENDERHAL_ASSERT(_expr)                                                       \
    MOS_ASSERT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _expr)

#define MHW_RENDERHAL_ASSERTMESSAGE(_message, ...)                                        \
    MOS_ASSERTMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _message, ##__VA_ARGS__)

#define MHW_RENDERHAL_NORMALMESSAGE(_message, ...)                                        \
    MOS_NORMALMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _message, ##__VA_ARGS__)

#define MHW_RENDERHAL_VERBOSEMESSAGE(_message, ...)                                       \
    MOS_VERBOSEMESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _message, ##__VA_ARGS__)

#define MHW_RENDERHAL_FUNCTION_ENTER                                                      \
    MOS_FUNCTION_ENTER(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL)

#define MHW_RENDERHAL_FUNCTION_EXIT                                                      \
    MOS_FUNCTION_EXIT(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, eStatus)

#define MHW_RENDERHAL_CHK_STATUS(_stmt)                                                   \
    MOS_CHK_STATUS(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _stmt)

#define MHW_RENDERHAL_CHK_STATUS_MESSAGE(_stmt, _message, ...)                        \
    MOS_CHK_STATUS_MESSAGE(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _stmt, _message, ##__VA_ARGS__)

#define MHW_RENDERHAL_CHK_NULL(_ptr)                                                      \
    MOS_CHK_NULL(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _ptr)

#define MHW_RENDERHAL_CHK_NULL_NO_STATUS(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _ptr)

#define MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(_ptr)                                            \
    MOS_CHK_NULL_NO_STATUS_RETURN(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _ptr)

#define MHW_RENDERHAL_CHK_NULL_RETURN(_ptr)                                            \
    MOS_CHK_NULL_RETURN(MOS_COMPONENT_CM, MOS_CM_SUBCOMP_RENDERHAL, _ptr)


#define MHW_RENDERHAL_UNUSED(x)                                                         \
    MOS_UNUSED(x)

//!
//! \brief  Kernel allocation control
//!
#define RENDERHAL_KERNEL_LOAD_FAIL         -1
#define RENDERHAL_KERNEL_ALLOCATION_FREE    0   // Kernel entry free
#define RENDERHAL_KERNEL_ALLOCATION_USED    1   // Kernel entry in use
#define RENDERHAL_KERNEL_ALLOCATION_LOCKED  2   // Kernel entry locked (no garbage collection)
#define RENDERHAL_KERNEL_ALLOCATION_REMOVED 3   // Kernel entry in use, but no longer loaded in ISH to make room for others
#define RENDERHAL_KERNEL_ALLOCATION_LOADING 4   // Kernel selected to be loaded (was stale or used)
#define RENDERHAL_KERNEL_ALLOCATION_STALE   5   // Kernel memory block became invalid, needs to be reloaded

//!
//! \brief  SSH defaults and limits
//!
#define RENDERHAL_SSH_INSTANCES            16
#define RENDERHAL_SSH_INSTANCES_MAX        64

#define RENDERHAL_SSH_BINDING_TABLES        1
#define RENDERHAL_SSH_BINDING_TABLES_MIN    1
#define RENDERHAL_SSH_BINDING_TABLES_MAX   16
#define RENDERHAL_SSH_BINDING_TABLE_ALIGN  64

#define RENDERHAL_SSH_SURFACE_STATES       40
#define RENDERHAL_SSH_SURFACE_STATES_MIN   16
#define RENDERHAL_SSH_SURFACE_STATES_MAX   256

#define RENDERHAL_SSH_SURFACES_PER_BT      64
#define RENDERHAL_SSH_SURFACES_PER_BT_MIN  4
#define RENDERHAL_SSH_SURFACES_PER_BT_MAX  256

//!
//! \brief  Default size of area for sync, debugging, performance collecting
//!
#define RENDERHAL_SYNC_SIZE_MIN             128
#define RENDERHAL_SYNC_SIZE_MAX             4096
#define RENDERHAL_SYNC_SIZE                 128

//!
//! \brief  Default number of media states (Dynamic GSH mode)
//!
#define RENDERHAL_MEDIA_STATES              16

//!
//! \brief  Default number of media IDs
//!
#define RENDERHAL_MEDIA_IDS                 16

//!
//! \brief  Max URB Size
//!
#define RENDERHAL_URB_SIZE_MAX              2048

//!
//! \brief  Interface Descriptor Entries
//!
#define RENDERHAL_INTERFACE_DESCRIPTOR_ENTRIES_MAX  64

//!
//! \brief  Max URB Entry Size
//!
#define RENDERHAL_URB_ENTRY_SIZE_MAX        (RENDERHAL_URB_SIZE_MAX - RENDERHAL_INTERFACE_DESCRIPTOR_ENTRIES_MAX)

//!
//! \brief  Max CURBE Allocation Size
//!
#define RENDERHAL_CURBE_SIZE_MAX            (RENDERHAL_URB_SIZE_MAX - RENDERHAL_INTERFACE_DESCRIPTOR_ENTRIES_MAX)

//!
//! \brief  Max Samplers
//!
#define RENDERHAL_SAMPLERS_AVS_MAX          8

//!
//! \brief  Default Samplers
//!
#define RENDERHAL_SAMPLERS                  16
#define RENDERHAL_SAMPLERS_VA               8

//!
//! \brief  Default CURBE size in GSH
//!         Use the size of composition kernel static param since it's the largest of all
//!
#define RENDERHAL_CURBE_SIZE                832  // MOS ALIGN CEIL(sizeof(GPGPU_WALKER_ISTAB_GMC_STATIC_DATA_G8), RENDERHAL_URB_BLOCK_ALIGN)

//!
//! \brief  Default number of kernels that may be cached in GSH
//!
#define RENDERHAL_KERNEL_COUNT             32

//!
//! \brief  Max number of kernels cached in GSH
//!
#define RENDERHAL_KERNEL_COUNT_MIN          2

//!
//! \brief  Default kernel heap size
//!
#define RENDERHAL_KERNEL_HEAP               2097152

//!
//! \brief  Min kernel heap size
//!
#define RENDERHAL_KERNEL_HEAP_MIN           65536
#define RENDERHAL_KERNEL_HEAP_MAX           2097152

//!
//! \brief  Default kernel block size (granularity for kernel allocation)
//!
#define RENDERHAL_KERNEL_BLOCK_SIZE         65536

//!
//! \brief  Default ISA ASM Debug Surface BTI
//!
#define RENDERHAL_ISA_ASM_SURFACE_BTI_DEFAULT   39

//!
//! \brief  Min kernel block size
//!
#define RENDERHAL_KERNEL_BLOCK_MIN          1024
#define RENDERHAL_KERNEL_BLOCK_MAX          65536

//!
//! \brief  Max number of Media Threads
//!
#define RENDERHAL_USE_MEDIA_THREADS_MAX     0

//!
//! \brief  Number and size of palettes
//!
#define RENDERHAL_PALETTE_COUNT             2
#define RENDERHAL_PALETTE_MAX               2

#define RENDERHAL_PALETTE_ENTRIES           256
#define RENDERHAL_PALETTE_ENTRIES_MAX       256

//!
//! \brief  SIP Size
//!
#define RENDERHAL_MAX_SIP_SIZE              0x4000

//!
//! \brief  Number of chroma keys
//!
#define RENDERHAL_CHROMA_KEY_COUNT          4
#define RENDERHAL_CHROMA_KEY_MAX            4

//!
//! \brief  Alignment
//!
#define RENDERHAL_KERNEL_BLOCK_ALIGN        64
#define RENDERHAL_URB_BLOCK_ALIGN           64
#define RENDERHAL_SYNC_BLOCK_ALIGN          128
#define RENDERHAL_CURBE_BLOCK_ALIGN         64

//!
//! \brief  Max number of Y_Uoffset size
//!
#define RENDERHAL_MAX_YV12_PLANE_Y_U_OFFSET_G9          16383

//!
//! \brief  Palette allocation id
//!
#define RENDERHAL_PALETTE_ID_ALLOCATE_ONLY  -2  // Allocate but don't load palette
#define RENDERHAL_PALETTE_ID_ALLOCATE_LOAD  -1  // Allocate and load palette

//!
//! \brief  Hw Interface defaults
//!
#define RENDERHAL_TIMEOUT_MS_DEFAULT        100
#define RENDERHAL_EVENT_TIMEOUT_MS          5

//!
//! \brief  Sampler State Indices
//!
#define RENDERHAL_SAMPLER_Y                 1
#define RENDERHAL_SAMPLER_U                 2
#define RENDERHAL_SAMPLER_V                 3
#define RENDERHAL_SAMPLER_8x8_AVS_Y         4
#define RENDERHAL_SAMPLER_8x8_AVS_U         8
#define RENDERHAL_SAMPLER_8x8_AVS_V         12

//*-----------------------------------------------------------------------------
//| MMIO register offsets used for the EU debug support
//*-----------------------------------------------------------------------------



#define MEDIASTATE_AVS_MAX_DERIVATIVE_4_PIXELS  7
#define MEDIASTATE_AVS_MAX_DERIVATIVE_8_PIXELS  20
#define MEDIASTATE_AVS_TRANSITION_AREA_4_PIXELS 4
#define MEDIASTATE_AVS_TRANSITION_AREA_8_PIXELS 5

enum GFX_COMMON_TOKEN_SUBOPCODE
{
    GFXSUBOP_BINDING_TABLE_STATE_TOKEN  = 0xFE,
    GFXSUBOP_SURFACE_STATE_TOKEN        = 0xFF
};

enum MEDIASTATE_AVS_SHARPNESS_LEVEL
{
    MEDIASTATE_AVS_SHARPNESS_LEVEL_SMOOTH = 0,
    MEDIASTATE_AVS_SHARPNESS_LEVEL_SHARP  = 255
};

enum ROTATION_MODE
{
    ROTATION_IDENTITY               = 0,
    ROTATION_90                     = 1,
    ROTATION_180                    = 2,
    ROTATION_270                    = 3,
};

// Render chroma siting vertical value
enum CHROMA_SITING_VDIRECTION
{
    CHROMA_SITING_VDIRECTION_0   = 0x0,
    CHROMA_SITING_VDIRECTION_1_4 = 0x1,
    CHROMA_SITING_VDIRECTION_1_2 = 0x2,
    CHROMA_SITING_VDIRECTION_3_4 = 0x3,
    CHROMA_SITING_VDIRECTION_1   = 0x4
};

// Render chroma siting horizontal value
enum CHROMA_SITING_UDIRECTION
{
    CHROMA_SITING_UDIRECTION_LEFT   = 0x0,
    CHROMA_SITING_UDIRECTION_CENTER = 0x1
};

enum GFX3DSTATE_TILEWALK
{
    GFX3DSTATE_TILEWALK_XMAJOR                 = 0,
    GFX3DSTATE_TILEWALK_YMAJOR                 = 1
};

enum MEDIA_STATE_DEBUG_COUNTER_CONTROL
{
    MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING       = 0,
    MEDIASTATE_DEBUG_COUNTER_FROZEN             = 1,
    MEDIASTATE_DEBUG_COUNTER_INITIALIZED_ONCE   = 2,
    MEDIASTATE_DEBUG_COUNTER_INITIALIZED_ALWAYS = 3
};

enum MEDIASTATE_BINDING_TABLE_STATE_TYPE
{
    MEDIASTATE_BTS_DEFAULT_TYPE             = 0,
    MEDIASTATE_BTS_DI_SAMPLE8x8_VME_TYPE    = 1
};

struct SURFACE_STATE_TOKEN_COMMON
{
    // DWORD 0
    union
    {
        struct
        {
            uint32_t   Length                  : 8;    // OP_LENGTH
            uint32_t                           : 8;
            uint32_t   InstructionSubOpcode    : 8;    // GFX3DSTATE_PIPELINED_SUBOPCODE
            uint32_t   InstructionOpcode       : 3;    // GFX_OPCODE
            uint32_t   InstructionPipeLine     : 2;    // INSTRUCTION_PIPELINE
            uint32_t   InstructionType         : 2;    // INSTRUCTION_TYPE
            uint32_t   Token                   : 1;    // bool
        };

        // DriverID for IMOLA patching
        struct
        {
            uint32_t   DriverID;
        };

        struct
        {
            uint32_t   Value;
        };
    } DW0;

    // DWORD 1
    union
    {
        struct
        {
            uint32_t   SurfaceStateHeapOffset  : 16;  // U16 32-byte aligned
            uint32_t   SurfaceAllocationIndex  : 16;  // U16
        };
        struct
        {
            uint32_t   Value;
        };
    } DW1;

    // DWORD 2
    union
    {
        struct
        {
            uint32_t   SurfaceOffset           : 32;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW2;

    // DWORD 3
    union
    {
        struct
        {
            uint32_t   RenderTargetEnable      : 1;  // bool
            uint32_t   YUVPlane                : 2;  // U2
            uint32_t   SurfaceStateType        : 1;  // U1
        uint32_t: 28;
        };
        struct
        {
            uint32_t   Value;
        };
    } DW3;

    // DWORD 4
    union
    {
        struct
        {
            uint32_t   SurfaceBaseAddress;                                 // SurfaceBaseAddress[31:12]
        };
        struct
        {
            uint32_t   Value;
        };
    } DW4;

    // DWORD 5
    union
    {
        struct
        {
            uint32_t   SurfaceBaseAddress64    : BITFIELD_RANGE(0, 15);    // SurfaceBaseAddress[47:32]
            uint32_t                           : BITFIELD_RANGE(16, 31);
        };
        struct
        {
            uint32_t   Value;
        };
    } DW5;

    void*   pResourceInfo;

};

extern const SURFACE_STATE_TOKEN_COMMON g_cInit_SURFACE_STATE_TOKEN_COMMON;

// Forward declarations
typedef struct _RENDERHAL_SURFACE    RENDERHAL_SURFACE, *PRENDERHAL_SURFACE;
typedef struct _RENDERHAL_INTERFACE  RENDERHAL_INTERFACE, *PRENDERHAL_INTERFACE;
typedef struct _RENDERHAL_SURFACE_STATE_ENTRY *PRENDERHAL_SURFACE_STATE_ENTRY;
typedef const struct _RENDERHAL_KERNEL_PARAM CRENDERHAL_KERNEL_PARAM, *PCRENDERHAL_KERNEL_PARAM;

//!
//! Structure RENDERHAL_SETTINGS
//! \brief RenderHal Settings - creation parameters for RenderHal
//!
typedef struct _RENDERHAL_SETTINGS
{
    int32_t iMediaStates;

    PRENDERHAL_DYN_HEAP_SETTINGS pDynSettings; // Dynamic State Heap Settings

} RENDERHAL_SETTINGS, *PRENDERHAL_SETTINGS;

//!
//! Enum RENDERHAL_COMPONENT
//! \brief RenderHal client component ID (for debugging/timing)
//!
typedef enum _RENDERHAL_COMPONENT
{
    RENDERHAL_COMPONENT_UNKNOWN = 0,
    RENDERHAL_COMPONENT_COMP,
    RENDERHAL_COMPONENT_DNDI,
    RENDERHAL_COMPONENT_VEBOX,
    RENDERHAL_COMPONENT_CM,
    RENDERHAL_COMPONENT_16ALIGN,
    RENDERHAL_COMPONENT_FAST1TON,
    RENDERHAL_COMPONENT_HDR,
    RENDERHAL_COMPONENT_COUNT_BASE,
    RENDERHAL_COMPONENT_RESERVED_NUM = 15,
    RENDERHAL_COMPONENT_COUNT
} RENDERHAL_COMPONENT;

//!
//! \brief Scaling Mode enum
//!
typedef enum _RENDERHAL_SCALING_MODE
{
    RENDERHAL_SCALING_NEAREST,
    RENDERHAL_SCALING_BILINEAR,
    RENDERHAL_SCALING_AVS
} RENDERHAL_SCALING_MODE;

//!
//! \brief Surface types enum
//!        IMPORTANT : SurfaceType_Layer[] must be updated to match this enum type
//!
typedef enum _RENDERHAL_SURFACE_TYPE
{
    RENDERHAL_SURF_NONE = 0,
    RENDERHAL_SURF_IN_BACKGROUND,
    RENDERHAL_SURF_IN_PRIMARY,
    RENDERHAL_SURF_IN_SUBSTREAM,
    RENDERHAL_SURF_IN_REFERENCE,
    RENDERHAL_SURF_OUT_RENDERTARGET,
    RENDERHAL_SURF_TYPE_COUNT                 //!< Keep this line at the end
} RENDERHAL_SURFACE_TYPE;

//!
//! \brief Batch buffer types enum
//!
typedef enum _RENDERHAL_BB_TYPE
{
    RENDERHAL_BB_TYPE_UNKNOWN        = 0, // Batch Buffer created by unknown client
    RENDERHAL_BB_TYPE_CM             = 6, // Batch Buffer created by Media Development Framework (CM) component
    RENDERHAL_BB_TYPE_COUNT
} RENDERHAL_BB_TYPE;

//!
//! \brief Sample Type enum
//!
typedef enum _RENDERHAL_SAMPLE_TYPE
{
    RENDERHAL_SAMPLE_PROGRESSIVE,
    RENDERHAL_SAMPLE_SINGLE_TOP_FIELD,
    RENDERHAL_SAMPLE_SINGLE_BOTTOM_FIELD,
    RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD,
    RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD,
    RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD,
    RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD,
    RENDERHAL_SAMPLE_INVALID                            //!< Keep this line at the end
} RENDERHAL_SAMPLE_TYPE;

//!
//! Structure RENDERHAL_GET_SURFACE_INFO
//! \brief Structure to retrieve Surface Infomation Parameters
//!
typedef struct _RENDERHAL_GET_SURFACE_INFO
{
    uint32_t          ArraySlice;
    uint32_t          MipSlice;
    MOS_S3D_CHANNEL   S3dChannel;
} RENDERHAL_GET_SURFACE_INFO, *PRENDERHAL_GET_SURFACE_INFO;

//!
//! \brief  Structure of power control info
//!
typedef struct _RENDERHAL_POWEROPTION
{
    uint16_t nSlice;                      //!< Number of slices to use: 0 (default), 1, 2...
    uint16_t nSubSlice;                   //!< Number of subslices to use: 0 (default), 1, 2...
    uint16_t nEU;                         //!< Number of EUs to use: 0 (default), 1, 2...
} RENDERHAL_POWEROPTION, *PRENDERHAL_POWEROPTION;

//!
//! Structure RENDERHAL_SURFACE
//! \brief RENDERHAL surface definition
//!
typedef struct _RENDERHAL_SURFACE
{
    MOS_SURFACE                 OsSurface;          //!< Surface provided by the client

    // Auxiliary rendering parameters
    RENDERHAL_SURFACE_TYPE      SurfType;           //!< Surface Type (context)
    RENDERHAL_SCALING_MODE      ScalingMode;        //!< Scaling Mode
    MHW_ROTATION                Rotation;           //!< Rotation Mode
    uint32_t                    ChromaSiting;       //!< Chroma Siting

    // Src/Dst rectangles
    RECT                        rcSrc;              //!< Source rectangle
    RECT                        rcDst;              //!< Destination rectangle
    RECT                        rcMaxSrc;           //!< Max source rectangle

    // Auxiliary VP parameters provided by client
    bool                        bDeinterlaceEnable; //!< Active Deinterlace messages
    bool                        bQueryVariance;     //!< enable variance query
    bool                        bInterlacedScaling; //!< Interlaced scaling
    void                        *pDeinterlaceParams; //!< Pointer to Deinterlacing parameters
    RENDERHAL_SAMPLE_TYPE       SampleType;         //!< Interlaced/Progressive sample type
    int32_t                     iPaletteID;         //!<Palette ID
} RENDERHAL_SURFACE , *PRENDERHAL_SURFACE;

//!
//! Structure RENDERHAL_OFFSET_OVERRIDE
//! \brief structure used to alloce Surface State overrides for Kernel
//!
typedef struct _RENDERHAL_OFFSET_OVERRIDE
{
    // Y plane adjustment
    int32_t iYOffsetAdjust;             //!< Surface Offset adjustment for Y plane
    int32_t iYOffsetX;                  //!< X-Offset override for Y plane
    int32_t iYOffsetY;                  //!< Y-Offset override for Y plane

    // UV plane overrides/adjustmenty
    int32_t iUVOffsetAdjust;            //!< Surface Offset adjustment for UV plane
    int32_t iUVOffsetX;                 //!< X-Offset override for UV plane
    int32_t iUVOffsetY;                 //!< Y-Offset override for UV plane
} RENDERHAL_OFFSET_OVERRIDE, *PRENDERHAL_OFFSET_OVERRIDE;

typedef enum _RENDERHAL_SURFACE_STATE_TYPE
{
    RENDERHAL_SURFACE_TYPE_INVALID  = 0,
    RENDERHAL_SURFACE_TYPE_G8       ,
    RENDERHAL_SURFACE_TYPE_G9       ,
    RENDERHAL_SURFACE_TYPE_G10      ,
    RENDERHAL_SURFACE_TYPE_ADV_G8   ,
    RENDERHAL_SURFACE_TYPE_ADV_G9   ,
    RENDERHAL_SURFACE_TYPE_ADV_G10
} RENDERHAL_SURFACE_STATE_TYPE, *PRENDERHAL_SURFACE_STATE_TYPE;

typedef enum _RENDERHAL_PLANE_DEFINITION
{
    RENDERHAL_PLANES_PL3            = 0,
    RENDERHAL_PLANES_NV12              ,
    RENDERHAL_PLANES_YUY2              ,
    RENDERHAL_PLANES_UYVY              ,
    RENDERHAL_PLANES_YVYU              ,
    RENDERHAL_PLANES_VYUY              ,
    RENDERHAL_PLANES_ARGB              ,
    RENDERHAL_PLANES_XRGB              ,
    RENDERHAL_PLANES_ABGR              ,
    RENDERHAL_PLANES_XBGR              ,
    RENDERHAL_PLANES_RGB16             ,
    RENDERHAL_PLANES_RGB24             ,
    RENDERHAL_PLANES_R16U              ,
    RENDERHAL_PLANES_R16S              ,
    RENDERHAL_PLANES_R32U              ,
    RENDERHAL_PLANES_R32S              ,
    RENDERHAL_PLANES_R32F              ,
    RENDERHAL_PLANES_V8U8              ,
    RENDERHAL_PLANES_R8G8_UNORM        ,
    RENDERHAL_PLANES_411P              ,
    RENDERHAL_PLANES_411R              ,
    RENDERHAL_PLANES_422H              ,
    RENDERHAL_PLANES_422V              ,
    RENDERHAL_PLANES_444P              ,
    RENDERHAL_PLANES_RGBP              ,
    RENDERHAL_PLANES_BGRP              ,

    RENDERHAL_PLANES_AI44_PALLETE_0    ,
    RENDERHAL_PLANES_IA44_PALLETE_0    ,
    RENDERHAL_PLANES_P8_PALLETE_0      ,
    RENDERHAL_PLANES_A8P8_PALLETE_0    ,
    RENDERHAL_PLANES_AI44_PALLETE_1    ,
    RENDERHAL_PLANES_IA44_PALLETE_1    ,
    RENDERHAL_PLANES_P8_PALLETE_1      ,
    RENDERHAL_PLANES_A8P8_PALLETE_1    ,

    RENDERHAL_PLANES_AYUV              ,
    RENDERHAL_PLANES_STMM              ,
    RENDERHAL_PLANES_L8                ,

    RENDERHAL_PLANES_PL3_ADV           ,
    RENDERHAL_PLANES_NV12_ADV          ,
    RENDERHAL_PLANES_YUY2_ADV          ,
    RENDERHAL_PLANES_UYVY_ADV          ,
    RENDERHAL_PLANES_YVYU_ADV          ,
    RENDERHAL_PLANES_VYUY_ADV          ,
    RENDERHAL_PLANES_ARGB_ADV          ,
    RENDERHAL_PLANES_ABGR_ADV          ,
    RENDERHAL_PLANES_AYUV_ADV           ,
    RENDERHAL_PLANES_STMM_ADV          ,
    RENDERHAL_PLANES_L8_ADV            ,
    RENDERHAL_PLANES_A8_ADV            ,
    RENDERHAL_PLANES_A8                ,
    RENDERHAL_PLANES_R8                ,
    RENDERHAL_PLANES_NV12_2PLANES      ,
    RENDERHAL_PLANES_NV12_2PLANES_ADV  ,
    RENDERHAL_PLANES_411P_ADV          ,
    RENDERHAL_PLANES_411R_ADV          ,
    RENDERHAL_PLANES_422H_ADV          ,
    RENDERHAL_PLANES_422V_ADV          ,
    RENDERHAL_PLANES_444P_ADV          ,
    RENDERHAL_PLANES_RGBP_ADV          ,
    RENDERHAL_PLANES_BGRP_ADV          ,
    RENDERHAL_PLANES_R16_UNORM         ,
    RENDERHAL_PLANES_Y8                ,
    RENDERHAL_PLANES_Y1                ,
    RENDERHAL_PLANES_Y16U              ,
    RENDERHAL_PLANES_Y16S              ,
    RENDERHAL_PLANES_A16B16G16R16      ,
    RENDERHAL_PLANES_A16B16G16R16_ADV  ,
    RENDERHAL_PLANES_R10G10B10A2       ,
    RENDERHAL_PLANES_R10G10B10A2_ADV   ,
    RENDERHAL_PLANES_B10G10R10A2       ,
    RENDERHAL_PLANES_L16               ,
    RENDERHAL_PLANES_NV21              ,
    RENDERHAL_PLANES_YV12              ,
    RENDERHAL_PLANES_P016              ,
    RENDERHAL_PLANES_P016_2PLANES_ADV  ,
    RENDERHAL_PLANES_P010              ,
    RENDERHAL_PLANES_P010_1PLANE       ,
    RENDERHAL_PLANES_P010_1PLANE_ADV   ,
    RENDERHAL_PLANES_IRW0              ,
    RENDERHAL_PLANES_IRW1              ,
    RENDERHAL_PLANES_IRW2              ,
    RENDERHAL_PLANES_IRW3              ,
    RENDERHAL_PLANES_A16B16G16R16F     ,
    RENDERHAL_PLANES_R16G16_UNORM      ,
    RENDERHAL_PLANES_R16_FLOAT         ,
    RENDERHAL_PLANES_A16R16G16B16F     ,
    RENDERHAL_PLANES_YUY2_2PLANES      ,
    RENDERHAL_PLANES_Y210_ADV          ,
    RENDERHAL_PLANES_Y210_RT           ,
    RENDERHAL_PLANES_Y210              ,
    RENDERHAL_PLANES_Y210_1PLANE_ADV   ,
    RENDERHAL_PLANES_R16G16_SINT       ,
    RENDERHAL_PLANES_R24_UNORM_X8_TYPELESS,
    RENDERHAL_PLANES_R32_FLOAT_X8X24_TYPELESS,
    RENDERHAL_PLANES_P208,
    RENDERHAL_PLANES_P208_1PLANE_ADV,
    RENDERHAL_PLANES_Y416_RT,
    RENDERHAL_PLANES_R32G32B32A32F,
    RENDERHAL_PLANES_Y8_ADV,

    RENDERHAL_PLANES_DEFINITION_COUNT
} RENDERHAL_PLANE_DEFINITION, *PRENDERHAL_PLANE_DEFINITION;

typedef enum _RENDERHAL_SS_BOUNDARY
{
    RENDERHAL_SS_BOUNDARY_SRCRECT = 0,                                                    // use for sources read via sampler
    RENDERHAL_SS_BOUNDARY_DSTRECT ,                                                       // use for RT by default
    RENDERHAL_SS_BOUNDARY_MAXSRCRECT,                                                     // use max source rect
    RENDERHAL_SS_BOUNDARY_ORIGINAL,                                                       // use for surfs that are not source or RT
} RENDERHAL_SS_BOUNDARY;

//!
//! \brief  Surface cache attributes
//!
#define RENDERHAL_MEMORY_OBJECT_CONTROL     uint32_t

typedef struct _RENDERHAL_KERNEL_PARAM
{
    int32_t             GRF_Count;                                              // Number of registers
    int32_t             BT_Count;                                               // Number of BT entries
    int32_t             Sampler_Count;                                          // Number of samplers
    int32_t             Thread_Count;                                           // Number of threads (max)
    int32_t             GRF_Start_Register;                                     // Start register
    int32_t             CURBE_Length;                                           // Constant URB length (in 256-bits)
    int32_t             block_width;                                            // Block width
    int32_t             block_height;                                           // Block height
    int32_t             blocks_x;                                               // Blocks in x
    int32_t             blocks_y;                                               // Blocks in y
} RENDERHAL_KERNEL_PARAM, *PRENDERHAL_KERNEL_PARAM;

typedef struct _RENDERHAL_CLONE_KERNEL_PARAM
{
    bool                isClone;                                                // If kernel has been cloned from another kernel (i.e. share same kernel binary)
    bool                isHeadKernel;                                           // If kernel is the "head" kernel (i.e. this allocation contains the kernel binary)
    int32_t             referenceCount;                                         // Number of cloned kernels currently pointing to the head kernel
    int32_t             cloneKernelID;                                          // Kernel ID of source kernel (i.e. kernel used to clone)
    int32_t             kernelBinaryAllocID;                                    // Allocation ID of the head kernel that contains the actual kernel binary
    uint32_t            dwOffsetForAllocID;                                     // Save the offset for this allocation ID, if clone using a different offset to point to head kernel binary
}RENDERHAL_CLONE_KERNEL_PARAM, *PRENDERHAL_CLONE_KERNEL_PARAM;

typedef struct tagKdll_CacheEntry Kdll_CacheEntry;
typedef struct _RENDERHAL_KRN_ALLOCATION *PRENDERHAL_KRN_ALLOCATION;
typedef struct _RENDERHAL_KRN_ALLOC_LIST *PRENDERHAL_KRN_ALLOC_LIST;

typedef struct _RENDERHAL_KRN_ALLOCATION
{
    int32_t                      iKID;                                          // Interface descriptor ID for the kernel (for 2nd level buffer)
    int32_t                      iKUID;                                         // Kernel Unique ID
    int32_t                      iKCID;                                         // Kernel Cache ID
    uint32_t                     dwSync;                                        // Kernel last sync (used to determine whether the kernel may be unloaded)
    FrameTrackerTokenFlat        trackerToken;                                  // Kernel last sync with multiple trackers
    uint32_t                     dwOffset;                                      // Kernel offset in GSH (from GSH base, 0 is KAC entry is available)
    int32_t                      iSize;                                         // Kernel block size in GSH (0 if not loaded)
    uint32_t                     dwFlags : 4;                                   // Kernel allocation flag
    uint32_t                     dwCount : 28;                                  // Kernel refresh counter
    RENDERHAL_KERNEL_PARAM       Params;                                        // Kernel parameters for RenderHal (states, rendering, etc)
    PMHW_KERNEL_PARAM            pMhwKernelParam;                               // Pointer to Kernel parameters for MHW
    Kdll_CacheEntry             *pKernelEntry;                                  // Pointer to Kernel entry for VP/KDLL
    RENDERHAL_CLONE_KERNEL_PARAM cloneKernelParams;                             // CM - Clone kernel information
    int32_t                 iAllocIndex;                                        // Kernel allocation index (index in kernel allocation table)

    // DSH - Dynamic list of kernel allocations
    PMHW_STATE_HEAP_MEMORY_BLOCK pMemoryBlock;                                  // Memory block in ISH
    PRENDERHAL_KRN_ALLOCATION    pNext;                                         // Next kernel in list
    PRENDERHAL_KRN_ALLOCATION    pPrev;                                         // Prev kernel in list
    PRENDERHAL_KRN_ALLOC_LIST    pList;                                         // Points to current list, regardless of flag
    uint32_t                     Reserved    : 16;                              // Reserved    - used for debugging
    uint32_t                                 : 16;                              //
    char                         *szKernelName;                                  // Kernel name - used for debugging
} RENDERHAL_KRN_ALLOCATION, *PRENDERHAL_KRN_ALLOCATION;

typedef struct _RENDERHAL_KRN_ALLOC_LIST
{
    PRENDERHAL_KRN_ALLOCATION pHead;                                            // Head of the list
    PRENDERHAL_KRN_ALLOCATION pTail;                                            // Tail of the list
    int32_t                   iCount;                                           // Number of objects
} RENDERHAL_KRN_ALLOC_LIST, *PRENDERHAL_KRN_ALLOC_LIST;

typedef struct _RENDERHAL_MEDIA_STATE *PRENDERHAL_MEDIA_STATE;

typedef struct _RENDERHAL_DYNAMIC_STATE *PRENDERHAL_DYNAMIC_STATE;

typedef struct _RENDERHAL_MEDIA_STATE
{
    // set at creation time
    uint32_t            dwOffset;                                               // Media State Base Address (from GSH base) - VFE
    int32_t             *piAllocation;                                           // Kernel allocation table

    // set at runtime
    uint32_t            dwSyncTag;                                              // Sync Tag
    FrameTrackerTokenFlat trackerToken;
    uint32_t            dwSyncCount;                                            // Number of sync tags
    int32_t             iCurbeOffset;                                           // Current CURBE Offset
    uint32_t            bBusy   : 1;                                            // 1 if the state is in use (must sync before use)
    uint32_t                    : 15;
    uint32_t            Reserved: 16;

    PRENDERHAL_MEDIA_STATE      pPrev;                                          // Next Media State
    PRENDERHAL_MEDIA_STATE      pNext;                                          // Previous Media State
    PRENDERHAL_DYNAMIC_STATE    pDynamicState;                                  // Dynamic states (nullptr if DSH not in use)
} RENDERHAL_MEDIA_STATE, *PRENDERHAL_MEDIA_STATE;

typedef struct _RENDERHAL_MEDIA_STATE_LIST
{
    PRENDERHAL_MEDIA_STATE  pHead;                                              // Head of the list
    PRENDERHAL_MEDIA_STATE  pTail;                                              // Tail of the list
    int32_t                 iCount;                                             // Number of objects
} RENDERHAL_MEDIA_STATE_LIST, *PRENDERHAL_MEDIA_STATE_LIST;

struct RENDERHAL_TR_RESOURCE {
    MOS_RESOURCE    osResource;
    bool            locked;
    uint32_t        *data;
    uint32_t        currentTrackerId;
};

typedef struct _RENDERHAL_STATE_HEAP_SETTINGS
{
    // Global GSH Allocation parameters
    int32_t             iSyncSize;                                              // Sync area for sync, perf, debug

    // Media State Allocation parameters
    int32_t             iMediaStateHeaps;                                       // Number of Media State heaps
    int32_t             iMediaIDs;                                              // Number of Media Interface Descriptors
    int32_t             iCurbeSize;                                             // Size of CURBE area
    int32_t             iSamplers;                                              // Number of Samplers/ID
    int32_t             iSamplersAVS;                                           // Number of AVS Samplers/ID
    int32_t             iSamplersVA;                                            // Number of Video Analytics Samplers/ID
    int32_t             iKernelCount;                                           // Number of Kernels that can be loaded
    int32_t             iKernelHeapSize;                                        // Size of GSH block for kernels
    int32_t             iKernelBlockSize;                                       // Kernel allocation block

    // Media VFE/ID configuration, limits
    int32_t             iPerThreadScratchSize;                                  // Size of the Scratch memory per Thread
    int32_t             iSipSize;                                               // SIP size

    // Surface State Heap Settings
    int32_t             iSurfaceStateHeaps;                                     // Number of SSH instances (same as iMediaStateHeaps)
    int32_t             iBindingTables;                                         // Number of BT per SSH instance
    int32_t             iSurfaceStates;                                         // Number of Surfaces per SSH
    int32_t             iSurfacesPerBT;                                         // Number of Surfaces per BT
    int32_t             iBTAlignment;                                           // BT Alignment size
} RENDERHAL_STATE_HEAP_SETTINGS, *PRENDERHAL_STATE_HEAP_SETTINGS;

typedef struct _RENDERHAL_STATE_HEAP
{
    //---------------------------
    // General State Heap
    //---------------------------
    uint32_t                dwSizeGSH;                                          // GSH size
    MOS_RESOURCE            GshOsResource;                                      // GSH OS Buffer
    bool                    bGshLocked;                                         // GSH is locked
    uint8_t                 *pGshBuffer;                                         // Pointer to GSH buffer data

    // Dynamic GSH sync
    uint32_t                dwOffsetSync;                                       // Offset of sync/perf data in GSH
    uint32_t                dwSizeSync;                                         // Size of sync data

    // Synchronization / Performance / Statistics
    volatile uint32_t       *pSync;                                              // Pointer to sync area (when locked)
    uint32_t                dwNextTag;                                          // Next sync tag value to use
    uint32_t                dwSyncTag;                                          // Last sync tag completed
    uint32_t                dwFrameId;                                          // Last frame id completed

    // Media states
    int32_t                 iCurMediaState;                                     // Current Media State Index
    int32_t                 iNextMediaState;                                    // Next Media State Index
    PRENDERHAL_MEDIA_STATE  pCurMediaState;                                     // Current Media state in use

    uint32_t                dwOffsetMediaID;                                    // Offset to Media IDs from Media State Base
    uint32_t                dwSizeMediaID;                                      // Size of each Media ID

    MHW_ID_ENTRY_PARAMS     CurIDEntryParams = {};                              // Parameters for current Interface Descriptor Entry

    // Performance capture
    uint32_t                dwOffsetStartTime;                                  // Offset to the start time of the media state
    uint32_t                dwStartTimeSize;                                    // Size of Start time
    uint32_t                dwOffsetEndTime;                                    // Offset to the end time of the media state
    uint32_t                dwEndTimeSize;                                      // Size of end time
    uint32_t                dwOffsetComponentID;                                // Render Component
    uint32_t                dwComponentIDSize;                                  // Render ComponentID size
    uint32_t                dwOffsetReserved;                                   // Reserved (Curbe should be 64 aligned)
    uint32_t                dwReservedSize;                                     // 64 - (start time + end time + component)

    uint32_t                dwOffsetCurbe;                                      // Offset to Media CURBE data from Media State Base
    uint32_t                dwSizeCurbe;                                        // Size of CURBE area

    uint32_t                dwOffsetSampler;                                    // Offset to Media Samplers from Media State Base
    uint32_t                dwSizeSampler;                                      // Size of Samplers

    uint32_t                dwOffsetSamplerIndirect;                            // Offset to Media Samplers Indirect State from Media State Base
    uint32_t                dwSizeSamplerIndirect;                              // Size of Samplers Indirect State

    uint32_t                dwOffsetSampler8x8Table;                            // Offset to Media Sampler State Table for AVS from Media State Base
    uint32_t                dwSizeSampler8x8Table;                              // Size of Sampler State Table for AVS

    uint32_t                dwOffsetSamplerVA;                                  // Offset to Video Analytics Samplers from Media State Base
    uint32_t                dwSizeSamplerVA;                                    // Size of VA Samplers

    uint32_t                dwOffsetSamplerAVS;                                 // Offset to 8x8 Samplers from Media State Base
    uint32_t                dwSizeSamplerAVS;                                   // Size of 8x8 Samplers

    PRENDERHAL_MEDIA_STATE  pMediaStates;                                       // Media state table

    // Dynamic Media states
    PMHW_MEMORY_POOL            pMediaStatesMemPool;                            // Media state memory allocations
    RENDERHAL_MEDIA_STATE_LIST  FreeStates;                                     // Free media state objects (pool)
    RENDERHAL_MEDIA_STATE_LIST  ReservedStates;                                 // Reserved media states
    RENDERHAL_MEDIA_STATE_LIST  SubmittedStates;                                // Submitted media states

    //---------------------------
    // Surface State Heap
    //---------------------------
    uint32_t                dwSizeSSH;                                          // SSH size
    MOS_RESOURCE            SshOsResource;                                      // SSH OS Buffer
    bool                    bSshLocked;                                         // SSH is locked
    uint8_t                 *pSshBuffer;                                         // Pointer to SSH buffer base
    uint32_t                dwSshIntanceSize;                                   // SSH instance size

    // BT size, offsets to BT/SS entries in SSH
    int32_t                 iBindingTableSize;                                  // Size of each BT (in bytes)
    int32_t                 iBindingTableOffset;                                // First BT offset in SSH buffer
    int32_t                 iSurfaceStateOffset;                                // First SS offset in SSH buffer

    // Array of Surface State control structures
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;

    // Current allocations
    int32_t                 iCurSshBufferIndex;                                 // Current SSH Buffer instance in the SSH heap
    int32_t                 iCurrentBindingTable;                               // Current BT
    int32_t                 iCurrentSurfaceState;                               // Current SS

    //---------------------------
    // Instruction State Heap
    //---------------------------
    uint32_t                dwSizeISH;                                          // ISH size
    MOS_RESOURCE            IshOsResource;                                      // ISH OS Buffer
    bool                    bIshLocked;                                         // ISH is locked
    uint8_t                 *pIshBuffer;                                         // Pointer to ISH buffer data
    uint32_t                dwKernelBase;                                       // Offset of kernels in ISH

    // Kernel Allocation
    int32_t                 iKernelSize;                                        // Kernel heap size
    int32_t                 iKernelUsed;                                        // Kernel heap used size
    uint8_t                 *pKernelLoadMap;                                     // Kernel load map
    uint32_t                dwAccessCounter;                                    // Incremented when a kernel is loaded/used, for dynamic allocation
    int32_t                 iKernelUsedForDump;                                 // The kernel size to be dumped in oca buffer.

    // Kernel Spill Area
    uint32_t                dwScratchSpaceSize;                                 // Size of the Scratch Area
    uint32_t                dwScratchSpaceBase;                                 // Base of the Scratch area

    // System Routine
    uint32_t                dwSipBase;                                          // Base of the System Routine

    // Arrays created dynamically
    PRENDERHAL_KRN_ALLOCATION   pKernelAllocation;                              // Kernel allocation table (or linked list)

    // Dynamic Kernel States
    PMHW_MEMORY_POOL               pKernelAllocMemPool;                         // Kernel states memory pool (mallocs)
    RENDERHAL_KRN_ALLOC_LIST       KernelAllocationPool;                        // Pool of kernel allocation objects
    RENDERHAL_KRN_ALLOC_LIST       KernelsSubmitted;                            // Kernel submission list
    RENDERHAL_KRN_ALLOC_LIST       KernelsAllocated;                            // kernel allocation list (kernels in ISH not currently being executed)                         
    CmHashTable                    kernelHashTable;                             // Kernel hash table for faster kernel search

} RENDERHAL_STATE_HEAP, *PRENDERHAL_STATE_HEAP;

typedef struct _RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS
{
    int32_t             iMaxMediaIDs;               // Maximum number of media interface descriptors
    int32_t             iMaxThreads;                // Maximum number of threads to be executed (0 = limited to HW threads) - for scratch space allocation
    int32_t             iMaxSpillSize;              // Maximum spill sizes among all kernels to be executed - used for scratch space allocation
    int32_t             iMaxCurbeOffset;            // Maximum offset reserved for CURBE
    int32_t             iMaxCurbeSize;              // Maximum size reserved for CURBE
    int32_t             iMaxSamplerIndex3D;         // Maximum 3D sampler index
    int32_t             iMaxSamplerIndexAVS;        // Maximum AVS sampler index
    int32_t             iMaxSamplerIndexConv;       // Maximum Conv sampler index
    int32_t             iMaxSamplerIndexMisc;       // Maximum Misc (VA) sampler index
    int32_t             iMax8x8Tables;              // Maximum Number of 8x8 tables per MediaID
} RENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS, *PRENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS;

typedef struct _RENDERHAL_INTERFACE_DESCRIPTOR_PARAMS
{
    int32_t             iMediaID;
    int32_t             iBindingTableID;
    int32_t             iCurbeOffset;
    int32_t             iCurbeLength;
    int32_t             iCrsThrdConstDataLn;
    int32_t             iNumberThreadsInGroup;
    bool                blGlobalBarrierEnable;
    bool                blBarrierEnable;
    int32_t             iSLMSize;
} RENDERHAL_INTERFACE_DESCRIPTOR_PARAMS, *PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS;

//!
//! \brief  ======== HW Abstraction Params ===================================
//!

typedef struct _RENDERHAL_SURFACE_STATE_PARAMS
{
    RENDERHAL_SURFACE_STATE_TYPE    Type                      : 5;              // Type of surface state
    uint32_t                        bRenderTarget             : 1;              // Render target flag
    uint32_t                        bVertStride               : 1;              // VL Stride
    uint32_t                        bVertStrideOffs           : 1;              // VL Stride Offset
    uint32_t                        bWidthInDword_Y           : 1;              // Width in dwords
    uint32_t                        bWidthInDword_UV          : 1;
    uint32_t                        bAVS                      : 1;              // AVS scaling
    RENDERHAL_SS_BOUNDARY           Boundary                  : 3;              // boundary to be aligned to rcSrc/rcDst/actual wd/ht
    uint32_t                        bWidth16Align             : 1;              // When VDI Walker is enabled, input surface width must be 16 aligned
    uint32_t                        b2PlaneNV12NeededByKernel : 1;              // Kernel needs surface state for both Y and UV
    uint32_t                        bForceNV12                : 1;              // Forces format to be treated as NV12. Only used in FRC.
    uint32_t                        b32MWColorFillKern        : 1;              // Flag for 32x32 Media walker + ColorFill kernel case
    uint32_t                        bVASurface                : 1;              // Value is 1 if VA surface, 0 if AVS surface
    uint32_t                        AddressControl            : 2;              // 0 clamp, 1 mirror, 2, 3 reserved
    uint32_t                        bWAUseSrcHeight           : 1;              // Surface state height use src height or surface height
    uint32_t                        bWAUseSrcWidth            : 1;              // Surface state width use src width or surface width
    uint32_t                        bForce3DLUTR16G16         : 1;              // Flag for 3D LUT source and targetsurface
    uint32_t                        bChromasiting             : 1;              // Flag for chromasiting use
    uint32_t                        bVmeUse                   : 1;              // Flag for VME use
    uint32_t                                                  : 5;
    RENDERHAL_MEMORY_OBJECT_CONTROL MemObjCtl;                                  // Caching attributes
} RENDERHAL_SURFACE_STATE_PARAMS, *PRENDERHAL_SURFACE_STATE_PARAMS;

typedef struct _RENDERHAL_SURFACE_STATE_ENTRY
{
    RENDERHAL_SURFACE_STATE_TYPE    Type;                                           // Type of surface state
    PMOS_SURFACE                    pSurface;                                       // Pointer to OS surface
    uint8_t                         *pSurfaceState;                                 // Pointer to Surface State
    SURFACE_STATE_TOKEN_COMMON      SurfaceToken;                                   // SurfaceS Token
    int32_t                         iSurfStateID;                                   // Surface state ID
    uint32_t                        dwSurfStateOffset;                              // Surface state offset (SSH)
    uint32_t                        dwFormat;                                       // Surface format
    uint32_t                        dwWidth;                                        // Surface width
    uint32_t                        dwHeight;                                       // Surface heigth
    uint32_t                        dwPitch;                                        // Surface pitch
    uint32_t                        dwQPitch;                                       // Surface qpitch
    uint32_t                        YUVPlane         :  2;                          // Plane
    uint32_t                        bAVS             :  1;                          // AVS scaling
    uint32_t                        bRenderTarget    :  1;                          // Render target flag
    uint32_t                        bVertStride      :  1;                          // VL Stride
    uint32_t                        bVertStrideOffs  :  1;                          // VL Stride Offset
    uint32_t                        bWidthInDword    :  1;                          // Width in dwords
    uint32_t                        bTiledSurface    :  1;                          // Tiled surface
    uint32_t                        bTileWalk        :  1;                          // Walk 0-X; 1-Y
    uint32_t                        bHalfPitchChroma :  1;                          // Half pitch for choma (AVS)
    uint32_t                        bInterleaveChroma:  1;                          // Interleaved chroma (AVS)
    uint32_t                        DirectionV       :  3;                          // UV direction         (AVS)
    uint32_t                        DirectionU       :  1;                          // UV direction         (AVS)
    uint32_t                        AddressControl   :  2;                          // 0 Clamp, 1 Mirror, 2, 3 resserved
    uint32_t                                         : 15;                          // RESERVED
    uint16_t                        wUXOffset;                                      // (X,Y) offset U (AVS/ADI)
    uint16_t                        wUYOffset;                                      //
    uint16_t                        wVXOffset;                                      // (X,Y) offset V (AVS/ADI)
    uint16_t                        wVYOffset;                                      //
} RENDERHAL_SURFACE_STATE_ENTRY, *PRENDERHAL_SURFACE_STATE_ENTRY;

//!
// \brief   Helper parameters used by Mhw_SendGenericPrologCmd and to initiate command buffer attributes
//!
typedef struct _RENDERHAL_GENERIC_PROLOG_PARAMS
{
    bool                            bMmcEnabled = 0;
    bool                            bEnableMediaFrameTracking = 0;
    uint32_t                        dwMediaFrameTrackingTag = 0;
    uint32_t                        dwMediaFrameTrackingAddrOffset = 0;
    PMOS_RESOURCE                   presMediaFrameTrackingSurface = nullptr;
    virtual ~_RENDERHAL_GENERIC_PROLOG_PARAMS() {}
} RENDERHAL_GENERIC_PROLOG_PARAMS, *PRENDERHAL_GENERIC_PROLOG_PARAMS;

//!
// \brief   Settings help to decide the value of L3 cache enabling register used for renderhal
//!
typedef struct _RENDERHAL_L3_CACHE_SETTINGS
{
    bool    bEnableSLM;     // Enable SLM cache configuration
    bool    bOverride;      // Override cache settings

    // Override values
    bool    bL3CachingEnabled;

    bool    bCntlRegOverride;
    bool    bCntlReg2Override;
    bool    bCntlReg3Override;
    bool    bSqcReg1Override;
    bool    bSqcReg4Override;
    bool    bLra1RegOverride;

    uint32_t dwCntlReg;
    uint32_t dwCntlReg2;
    uint32_t dwCntlReg3;
    uint32_t dwSqcReg1;
    uint32_t dwSqcReg4;
    uint32_t dwLra1Reg;
} RENDERHAL_L3_CACHE_SETTINGS, *PRENDERHAL_L3_CACHE_SETTINGS;

//!
// \brief   Settings of Predication
//!
typedef struct _RENDERHAL_PREDICATION_SETTINGS
{
    MOS_RESOURCE            *pPredicationResource;    // Resource for predication
    MOS_RESOURCE            *ptempPredicationBuffer;  // Additional temp buffer for Predication due to the limitation of Cond_BB_End
    uint64_t                predicationResOffset;     // Offset for Predication resource
    bool                    predicationNotEqualZero;  // Predication mode
    bool                    predicationEnabled;       // Indicates whether or not Predication is enabled
} RENDERHAL_PREDICATION_SETTINGS;

//!
// \brief   Settings of SetMarker
//!
typedef struct _RENDERHAL_SETMARKER_SETTINGS
{
    MOS_RESOURCE            *pSetMarkerResource;      // Resource for SetMarker
    bool                    setMarkerEnabled;         // Indicates whether or not SetMarker is enabled
    uint32_t                setMarkerNumTs;           // Number Timestamp for SetMarker
} RENDERHAL_SETMARKER_SETTINGS;

typedef MhwMiInterface *PMHW_MI_INTERFACE;
//!
// \brief   Hardware dependent render engine interface
//!
typedef struct _RENDERHAL_INTERFACE
{
    // MOS/MHW Interfaces
    PMOS_INTERFACE               pOsInterface;
    MhwCpInterface               *pCpInterface;
    PXMHW_STATE_HEAP_INTERFACE   pMhwStateHeap;
    PMHW_MI_INTERFACE            pMhwMiInterface;
    MhwRenderInterface           *pMhwRenderInterface;

    // RenderHal State Heap
    PRENDERHAL_STATE_HEAP        pStateHeap;
    uint32_t                     dwStateHeapSize;

    // Linked list of batch buffers for synchronization
    PMHW_BATCH_BUFFER            pBatchBufferList;                              // List of BB submitted

    PMHW_MEMORY_POOL             pBatchBufferMemPool;                           // Batch Buffer memory allocations (malloc)
    PMHW_BATCH_BUFFER_LIST       BatchBufferPool;                               // Pool of BB objects   (no GFX buffer)
    PMHW_BATCH_BUFFER_LIST       BatchBuffersAllocated;                         // List of BB allocated (not executing, backed by GFX buffer)

    // Auxiliary
    PLATFORM                     Platform;
    MEDIA_FEATURE_TABLE          *pSkuTable;
    MEDIA_WA_TABLE               *pWaTable;

    // Hardware dependent parameters
    MHW_VFE_SCOREBOARD           VfeScoreboard;
    PCMHW_SURFACE_PLANES         pPlaneDefinitions;

    // Settings and capabilities
    PMHW_RENDER_ENGINE_CAPS       pHwCaps;                                      // HW Capabilities
    PMHW_RENDER_STATE_SIZES       pHwSizes;                                     // Sizes of HW commands/states
    RENDERHAL_STATE_HEAP_SETTINGS StateHeapSettings;                            // State Heap Settings
    RENDERHAL_DYN_HEAP_SETTINGS   DynamicHeapSettings;                          // Dynamic State Heap Settings

    // MHW parameters
    MHW_STATE_BASE_ADDR_PARAMS   StateBaseAddressParams;
    MHW_SIP_STATE_PARAMS         SipStateParams;
    MHW_WALKER_MODE              MediaWalkerMode;                               // Media object walker mode from Regkey: repel, dual mode, quad mode

    RENDERHAL_SURFACE_STATE_TYPE SurfaceTypeDefault;                            // Surface State type default
    RENDERHAL_SURFACE_STATE_TYPE SurfaceTypeAdvanced;                           // Surface State type advanced

    RENDERHAL_L3_CACHE_SETTINGS  L3CacheSettings;                               // L3 Cache settings

    bool                        bEnableYV12SinglePass;                          // Enabled YV12 single pass in 3D sampler
    bool                        bEnableP010SinglePass;                          // Enabled P010 single pass in sampler
    bool                        bSIPKernel;                                     // SIP loaded
    bool                        bCSRKernel;                                     // CSR loaded
    bool                        bTurboMode;                                     // Turbo mode info to pass in cmdBuf
    bool                        bVDIWalker;                                     // VDI Walker info from Regkey
    bool                        bRequestSingleSlice;                            // Single Slice Request flag
    bool                        bEUSaturationNoSSD;                             // No slice shutdown, must request 2 slices [CM EU saturation on]
    bool                        bEnableGpgpuMidBatchPreEmption;                 // Middle Batch Buffer Preemption
    bool                        bEnableGpgpuMidThreadPreEmption;                // Middle Thread Preemption
    bool                        bComputeContextInUse;                           // Compute Context use for media

    uint32_t                    dwMaskCrsThdConDataRdLn;                        // Unifies pfnSetupInterfaceDescriptor for g75,g8,...
    uint32_t                    dwMinNumberThreadsInGroup;                      // Unifies pfnSetupInterfaceDescriptor for g75,g8,...
    uint32_t                    dwCurbeBlockAlign;                              // Unifies pfnLoadCurbeData - Curbe Block Alignment
    uint32_t                    dwScratchSpaceMaxThreads;                       // Unifies pfnGetScratchSpaceSize - Threads used for scratch space calculation
    uint32_t                    dwSamplerAvsIncrement;                          // Unifies pfnSetSamplerStates

    const void                  *sseuTable;                                     // pointer of const VphalSseuSetting table on a platform

    uint32_t                    dwIndirectHeapSize;
    uint32_t                    dwTimeoutMs;
    int32_t                     iMaxPalettes;
    int32_t                     iMaxPaletteEntries;
    MHW_PALETTE_PARAMS          Palette[RENDERHAL_PALETTE_MAX];

    int32_t                     iMaxChromaKeys;
    int32_t                     iChromaKeyCount;
    MHW_CHROMAKEY_PARAMS        ChromaKey[RENDERHAL_CHROMA_KEY_MAX];

    bool                        bHasCombinedAVSSamplerState;

    // GD2 kernel debugging
    bool                        bIsaAsmDebugEnable;
    uint8_t                     cIsaAsmDebugSurfaceBTI;
    RENDERHAL_SURFACE           IsaAsmDebugSurface;

    // Performance collection
    bool                        bKerneltimeDump;
    double                      kernelTime[RENDERHAL_COMPONENT_COUNT];

    // Auxiliary data - for debugging purposes
    int32_t                     iMediaStatesInUse;  // Media states in use
    int32_t                     iKernelsInUse;      // Kernels in use
    int32_t                     iBuffersInUse;      // BB in use

    // Power option to control slice/subslice/EU shutdown
    RENDERHAL_POWEROPTION       PowerOption;

    // Indicates whether it's MDF load or not
    bool                        IsMDFLoad;

    bool                        bDynamicStateHeap;        //!< Indicates that DSH is in use


    FrameTrackerProducer        trackerProducer;        // Resource to mark command buffer completion
    RENDERHAL_TR_RESOURCE       veBoxTrackerRes;        // Resource to mark command buffer completion
    uint32_t                    currentTrackerIndex;    // Record the tracker index

    HeapManager                 *dgsheapManager;        // Dynamic general state heap manager

#if (_DEBUG || _RELEASE_INTERNAL)
    // Dump state for VP debugging
    void                        *pStateDumper;
#endif

    // Pointer to vphal oca dumper object to dump vphal parameters.
    void                        *pVphalOcaDumper;

    // Predication
    RENDERHAL_PREDICATION_SETTINGS PredicationParams;   //!< Predication
    MOS_RESOURCE                   PredicationBuffer;   //!< Predication buffer

    // CSC Coefficient
    bool                           bCmfcCoeffUpdate;    //!< CMFC CSC Coefficient Surface update flag
    int32_t                        iKernelAllocationID; //!< CMFC CSC Kernel Allocation ID
    PMOS_RESOURCE                  pCmfcCoeffSurface;   //!< CMFC CSC Coefficient Surface

    // SetMarker
    RENDERHAL_SETMARKER_SETTINGS SetMarkerParams;   //!< SetMarker

    // Indicates whether it's AVS or not
    bool                        bIsAVS;

    bool                        isMMCEnabled;

    MediaPerfProfiler               *pPerfProfiler = nullptr;  //!< Performance data profiler

    //---------------------------
    // HW interface functions
    //---------------------------
    MOS_STATUS (* pfnInitialize)(
                PRENDERHAL_INTERFACE     pRenderHal,
                PRENDERHAL_SETTINGS      pSettings);

    MOS_STATUS (* pfnDestroy) (
                PRENDERHAL_INTERFACE     pRenderHal);

    MOS_STATUS (* pfnReset) (
                PRENDERHAL_INTERFACE     pRenderHal);

    //---------------------------
    // State Heap Functions
    //---------------------------
    MOS_STATUS (* pfnAllocateStateHeaps) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_STATE_HEAP_SETTINGS  pSettings);

    MOS_STATUS (* pfnFreeStateHeaps) (
                PRENDERHAL_INTERFACE     pRenderHal);

    MOS_STATUS (* pfnRefreshSync) (
                PRENDERHAL_INTERFACE     pRenderHal);

    //---------------------------
    // SSH, surface states
    //---------------------------
    MOS_STATUS (* pfnAssignSshInstance) (
                PRENDERHAL_INTERFACE    pRenderHal);

    MOS_STATUS (* pfnGetSurfaceStateEntries) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_SURFACE              pRenderHalSurface,
                PRENDERHAL_SURFACE_STATE_PARAMS pParams,
                int32_t                         *piNumEntries,
                PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries);

    MOS_STATUS (* pfnSetupSurfaceState) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_SURFACE              pRenderHalSurface,
                PRENDERHAL_SURFACE_STATE_PARAMS pParams,
                int32_t                         *piNumEntries,
                PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
                PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride);

    MOS_STATUS (*pfnSetupBufferSurfaceState) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_SURFACE              pRenderHalSurface,
                PRENDERHAL_SURFACE_STATE_PARAMS pParams,
                PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntry);

    MOS_STATUS (* pfnAssignSurfaceState) (
                PRENDERHAL_INTERFACE            pRenderHal,
                RENDERHAL_SURFACE_STATE_TYPE    Type,
                PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntry);

    void (* pfnGetAlignUnit) (
                uint16_t                        *pwWidthAlignUnit,
                uint16_t                        *pwHeightAlignUnit,
                PRENDERHAL_SURFACE              pRenderHalSurface);

    void (* pfnAdjustBoundary) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_SURFACE              pRenderHalSurface,
                RENDERHAL_SS_BOUNDARY           Boundary,
                uint32_t                        *pdwSurfaceWidth,
                uint32_t                        *pdwSurfaceHeight);

    uint32_t (* pfnSetSurfacesPerBT) (
                PRENDERHAL_INTERFACE            pRenderHal,
                uint32_t                        dwSurfacesPerBT);

    uint16_t (* pfnCalculateYOffset) (
                PMOS_INTERFACE                  pOsInterface, 
                PMOS_RESOURCE                   pOsResource);

    MOS_STATUS (* pfnAssignBindingTable) (
                PRENDERHAL_INTERFACE            pRenderHal,
                int32_t                         *piBindingTable);

    MOS_STATUS (* pfnBindSurfaceState) (
                PRENDERHAL_INTERFACE            pRenderHal,
                int32_t                         iBindingTableIndex,
                int32_t                         iBindingTableEntry,
                PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry);

    uint32_t (* pfnGetSurfaceMemoryObjectControl) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_SURFACE_STATE_PARAMS pParams);

    //---------------------------
    // State Setup - HW + OS Specific
    //---------------------------
    MOS_STATUS (* pfnSetupSurfaceStatesOs) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_SURFACE_STATE_PARAMS pParams,
                PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceStateEntry);

    //---------------------------
    // Batch Buffer
    //---------------------------
    MOS_STATUS (* pfnAllocateBB) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMHW_BATCH_BUFFER           pBatchBuffer,
                int32_t                     iSize);

    MOS_STATUS (* pfnFreeBB) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMHW_BATCH_BUFFER           pBatchBuffer);

    MOS_STATUS (* pfnLockBB) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMHW_BATCH_BUFFER           pBatchBuffer);

    MOS_STATUS (* pfnUnlockBB) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMHW_BATCH_BUFFER           pBatchBuffer);

    //---------------------------
    // Media State
    //---------------------------
    PRENDERHAL_MEDIA_STATE (* pfnAssignMediaState) (
                PRENDERHAL_INTERFACE        pRenderHal,
                RENDERHAL_COMPONENT         componentID);

    //---------------------------
    // Allocation
    //---------------------------
    MOS_STATUS (* pfnEnablePalette) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iPaletteID,
                int32_t                     iPaletteSize);

    MOS_STATUS (* pfnAllocatePaletteID) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     *pPaletteID);

    MOS_STATUS (* pfnFreePaletteID) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     *pPaletteID);

    MOS_STATUS (* pfnGetPaletteEntry) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iPaletteID,
                int32_t                     iInNumEntries,
                int32_t                     *piOutNumEntries,
                void                        **pPaletteData);

    int32_t (* pfnAllocateChromaKey) (
                PRENDERHAL_INTERFACE        pRenderHal,
                uint32_t                    dwLow,
                uint32_t                    dwHigh);

    int32_t (* pfnLoadCurbeData) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_MEDIA_STATE      pMediaState,
                void                        *pData,
                int32_t                     iSize);

    MOS_STATUS (* pfnSetSamplerStates) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iMediaID,
                PMHW_SAMPLER_STATE_PARAM    pSamplerParams,
                int32_t                     iSamplers);

    int32_t (* pfnAllocateMediaID) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iKernelAllocationID,
                int32_t                     iBindingTableID,
                int32_t                     iCurbeOffset,
                int32_t                     iCurbeLength,
                int32_t                     iCrsThrdConstDataLn,
                PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams);

    int32_t (* pfnGetMediaID) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_MEDIA_STATE      pMediaState,
                PRENDERHAL_KRN_ALLOCATION   pKernelAllocation);

    MOS_STATUS (* pfnSetupInterfaceDescriptor) (
                PRENDERHAL_INTERFACE                   pRenderHal,
                PRENDERHAL_MEDIA_STATE                 pMediaState,
                PRENDERHAL_KRN_ALLOCATION              pKernelAllocation,
                PRENDERHAL_INTERFACE_DESCRIPTOR_PARAMS pInterfaceDescriptorParams);

    uint32_t (* pfnEncodeSLMSize)(PRENDERHAL_INTERFACE pRenderHal, uint32_t SLMSize);

    //---------------------------
    // Kernels
    //---------------------------
    int32_t (* pfnLoadKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PCRENDERHAL_KERNEL_PARAM    pParameters,
                PMHW_KERNEL_PARAM           pKernel,
                Kdll_CacheEntry             *pKernelEntry);

    MOS_STATUS (* pfnUnloadKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iKernelAllocationID);

    void (* pfnResetKernels) (
                PRENDERHAL_INTERFACE        pRenderHal);

    void (* pfnTouchKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iKernelAllocationID);

    int32_t (* pfnGetKernelOffset) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iKernelAllocationIndex);

    MOS_STATUS (* pfnUnregisterKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_KRN_ALLOCATION   pKernelAllocation);

    //---------------------------
    // New Dynamic State Heap interfaces
    //---------------------------
    MOS_STATUS(*pfnAssignSpaceInStateHeap)(
        uint32_t              trackerIndex,
        FrameTrackerProducer  *trackerProducer,
        HeapManager           *heapManager,
        MemoryBlock           *block,
        uint32_t               size);

    PRENDERHAL_MEDIA_STATE (* pfnAssignDynamicState) (
                PRENDERHAL_INTERFACE                  pRenderHal,
                PRENDERHAL_DYNAMIC_MEDIA_STATE_PARAMS pParams,
                RENDERHAL_COMPONENT                   componentID);

    MOS_STATUS (* pfnReleaseDynamicState) (
                PRENDERHAL_INTERFACE                  pRenderHal,
                PRENDERHAL_MEDIA_STATE                pMediaState);

    MOS_STATUS (* pfnSubmitDynamicState) (
                PRENDERHAL_INTERFACE                  pRenderHal,
                PRENDERHAL_MEDIA_STATE                pMediaState);

    int32_t (* pfnAllocateDynamicMediaID) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_KRN_ALLOCATION   pKernelAllocation,
                int32_t                     iBindingTableID,
                int32_t                     iCurbeOffset,
                int32_t                     iCurbeLength,
                int32_t                     iCrsThrdConstDataLn,
                PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams);

    PRENDERHAL_KRN_ALLOCATION (* pfnLoadDynamicKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PCRENDERHAL_KERNEL_PARAM    pParameters,
                PMHW_KERNEL_PARAM           pKernel,
                uint32_t                    *pdwLoaded);

    PRENDERHAL_KRN_ALLOCATION (* pfnSearchDynamicKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iKernelUniqueID,
                int32_t                     iCacheID);

    PRENDERHAL_KRN_ALLOCATION (* pfnAllocateDynamicKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iKernelUniqueID,
                int32_t                     iCacheID);

    MOS_STATUS (* pfnUnloadDynamicKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_KRN_ALLOCATION   pKernelAllocation);

    MOS_STATUS (* pfnRefreshDynamicKernels) (
                PRENDERHAL_INTERFACE        pRenderHal,
                uint32_t                    dwSpaceNeeded,
                uint32_t                    *pdwSizes,
                int32_t                     iCount);

    void (* pfnResetDynamicKernels) (
                PRENDERHAL_INTERFACE        pRenderHal);

    void (* pfnTouchDynamicKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_KRN_ALLOCATION   pKernelAllocation);

    MOS_STATUS (* pfnExpandKernelStateHeap)(
                PRENDERHAL_INTERFACE        pRenderHal,
        uint32_t                            dwAdditionalKernelSpaceNeeded);

    //---------------------------
    // ISA ASM Debug support functions
    //---------------------------
    int32_t (* pfnLoadDebugKernel)(
                PRENDERHAL_INTERFACE        pRenderHal,
                PMHW_KERNEL_PARAM           pKernel);

    MOS_STATUS (* pfnLoadSipKernel) (
                PRENDERHAL_INTERFACE        pRenderHal,
                void                        *pSipKernel,
                uint32_t                    dwSipSize);

    MOS_STATUS (* pfnSendSipStateCmd) (
        PRENDERHAL_INTERFACE                pRenderHal,
        PMOS_COMMAND_BUFFER                 pCmdBuffer);

    //---------------------------
    // HW interface configuration functions
    //---------------------------
    MOS_STATUS (* pfnSetVfeStateParams) (
                PRENDERHAL_INTERFACE        pRenderHal,
                uint32_t                    dwDebugCounterControl,
                uint32_t                    dwMaximumNumberofThreads,
                uint32_t                    dwCURBEAllocationSize,
                uint32_t                    dwURBEntryAllocationSize,
                PMHW_VFE_SCOREBOARD         pScoreboardParams);

    bool (* pfnGetMediaWalkerStatus) (
                PRENDERHAL_INTERFACE        pRenderHal);

    //---------------------------
    // Command buffer programming functions
    //---------------------------
    MOS_STATUS (* pfnSendStateBaseAddress) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnSendMediaStates) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer,
                PMHW_WALKER_PARAMS          pWalkerParams,
                PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams);

    MOS_STATUS (* pfnInitCommandBuffer) (
                PRENDERHAL_INTERFACE                pRenderHal,
                PMOS_COMMAND_BUFFER                 pCmdBuffer,
                PRENDERHAL_GENERIC_PROLOG_PARAMS    pGenericPrologParams);

    MOS_STATUS (* pfnSendSurfaces) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnSendSyncTag) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (*pfnSendCscCoeffSurface) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer,
                PMOS_RESOURCE               presCscCoeff,
                Kdll_CacheEntry             *pKernelEntry);

    void       (* pfnSetupPrologParams) (
                PRENDERHAL_INTERFACE             renderHal,
                RENDERHAL_GENERIC_PROLOG_PARAMS  *prologParams,
                PMOS_RESOURCE                    osResource,
                uint32_t                         offset,
                uint32_t                         tag);

    // Samplers and other states
    MOS_STATUS (*pfnGetSamplerOffsetAndPtr) (
                PRENDERHAL_INTERFACE        pRenderHal,
                int32_t                     iMediaID,
                int32_t                     iSamplerID,
                PMHW_SAMPLER_STATE_PARAM    pSamplerParams,
                uint32_t                    *pdwSamplerOffset,
                void                        **ppSampler);

    MOS_STATUS (* pfnSendCurbeLoad) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnSendMediaIdLoad) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnSendChromaKey) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnSendPalette) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnSendSurfaceStateEntry) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PMOS_COMMAND_BUFFER             pCmdBuffer,
                PMHW_SURFACE_STATE_SEND_PARAMS  pParams);

    MOS_STATUS (* pfnSetSurfaceStateToken)(
                PRENDERHAL_INTERFACE        pRenderHal,
                PMHW_SURFACE_TOKEN_PARAMS   pParams,
                void                        *pSurfaceStateToken);

    MOS_STATUS (* pfnSetSurfaceStateBuffer)(
                PRENDERHAL_INTERFACE        pRenderHal,
                PMHW_RCS_SURFACE_PARAMS     pParams,
                void                        *pSurfaceState);

    //-----------------------------
    // Slice Shutdown Mode function
    //-----------------------------
    void (* pfnSetSliceShutdownMode) (
                PRENDERHAL_INTERFACE        pRenderHal,
                bool                        bMode);

    //-----------------------------
    // General Slice Shut Down Mode function
    //-----------------------------
    void( *pfnSetPowerOptionMode ) (
        PRENDERHAL_INTERFACE                pRenderHal,
        PRENDERHAL_POWEROPTION              pMode);

    //-----------------------------
    // Enable Middle Batch Buffer Preemption
    //-----------------------------
    void (* pfnEnableGpgpuMiddleBatchBufferPreemption) (
                PRENDERHAL_INTERFACE        pRenderHal);

    //-----------------------------
    // Enable Middle Thread Preemption
    //-----------------------------
    void (* pfnEnableGpgpuMiddleThreadPreemption) (
                PRENDERHAL_INTERFACE        pRenderHal);

    //---------------------------
    // Generic HAL Layer Commands and State Functions
    //---------------------------
    void (* pfnConvertToNanoSeconds) (
                PRENDERHAL_INTERFACE        pRenderHal,
                uint64_t                    iTicks,
                uint64_t                    *piNs);

    MOS_STATUS (* pfnSendRcsStatusTag) (
               PRENDERHAL_INTERFACE         pRenderHal,                                             // [in] Hardware interface
               PMOS_COMMAND_BUFFER          pCmdBuffer);                                            // [in] Command Buffer

    MOS_STATUS (* pfnSendTimingData) (
                PRENDERHAL_INTERFACE        pRenderHal,                                             // [in] Hardware interface
                PMOS_COMMAND_BUFFER         pCmdBuffer,                                             // [in] Command Buffer
                bool                        bStartTime);                                            // [in] Start Timestamp flag

    uint32_t (* pfnGetScratchSpaceSize)(
               PRENDERHAL_INTERFACE         pRenderHal,                                             // [in] Hardware interface
               uint32_t                     iPerThreadScratchSpaceSize);                            // [in] Per thread scrach space size

    bool (* pfnIs2PlaneNV12Needed) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_SURFACE          pRenderHalSurface,
                RENDERHAL_SS_BOUNDARY       Boundary);

    uint8_t (* pfnSetChromaDirection) (
                PRENDERHAL_INTERFACE        pRenderHal,
                PRENDERHAL_SURFACE          pRenderHalSurface);

    bool(*pfnPerThreadScratchSpaceStart2K) (
                PRENDERHAL_INTERFACE        pRenderHal);

    //---------------------------
    // Overwrite L3 Cache control register
    //---------------------------
    MOS_STATUS (* pfnEnableL3Caching) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings);

    MOS_STATUS(*pfnSetCacheOverrideParams) (
                PRENDERHAL_INTERFACE            pRenderHal,
                PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
                bool                            bEnableSLM);

    //-----------------------------
    //Platform related interface
    XRenderHal_Platform_Interface           *pRenderHalPltInterface;

} RENDERHAL_INTERFACE;

//!
//! \brief   Functions
//!

//!
//! \brief    Init Interface
//! \details  Initializes Render Hal Interface structure, responsible for HW
//!           abstraction of Render Engine for MDF/VP
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    MhwCpInterface** ppCpInterface
//!           [in/out] Pointer of pointer to MHW CP Interface Structure, which 
//!           is created during renderhal initialization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_UNKNOWN : Invalid parameters
//!
MOS_STATUS RenderHal_InitInterface(
    PRENDERHAL_INTERFACE        pRenderHal,
    MhwCpInterface              **ppCpInterface,
    PMOS_INTERFACE              pOsInterface);

//!
//! \brief    Init Interface using Dynamic State Heap
//! \details  Initializes RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    MhwCpInterface** ppCpInterface
//!           [in/out] Pointer of pointer to MHW CP Interface Structure, which 
//!           is created during renderhal initialization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_UNKNOWN : Invalid parameters
//!
MOS_STATUS RenderHal_InitInterface_Dynamic(
    PRENDERHAL_INTERFACE        pRenderHal,
    MhwCpInterface              **ppCpInterface,
    PMOS_INTERFACE              pOsInterface);

//!
//! \brief    Get Pixels Per Sample
//! \details  Get Number of Pixels per Dataport Sample
//! \param    MOS_FORMAT format
//!           [in] Surface Format
//! \param    uint32_t *pdwPixelsPerSampleUV
//!           [in] Pointer to dwPixelsPerSampleUV
//! \return   void
//!
void RenderHal_GetPixelsPerSample(
    MOS_FORMAT                  format,
    uint32_t                    *pdwPixelsPerSampleUV);

//!
//! \brief    Set Surface for HW Access
//! \details  Common Function for setting up surface state
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Render Hal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams
//!           [in] Pointer to Surface Params
//! \param    int32_t iBindingTable
//!           [in] Binding Table to bind surface
//! \param    int32_t iBTEntry
//!           [in] Binding Table Entry index
//! \param    bool bWrite
//!           [in] Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS RenderHal_SetSurfaceForHwAccess(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
    int32_t                         iBindingTable,
    int32_t                         iBTEntry,
    bool                            bWrite);

//!
//! \brief    Set Buffer Surface for HW Access
//! \details  Common Function for setting up buffer surface state
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Render Hal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams
//!           [in] Pointer to Surface Params
//! \param    int32_t iBindingTable
//!           [in] Binding Table to Bind Surface
//! \param    int32_t iBTEntry
//!           [in] Binding Table Entry index
//! \param    bool bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS RenderHal_SetBufferSurfaceForHwAccess(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams,
    int32_t                         iBindingTable,
    int32_t                         iBTEntry,
    bool                            bWrite);

//!
//! \brief    Get Surface Info from OsResource
//! \details  Update surface info in PRENDERHAL_SURFACE based on allocated OsResource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to MOS_INTERFACE
//! \param    PRENDERHAL_GET_SURFACE_INFO pInfo
//!           [in] Pointer to RENDERHAL_GET_SURFACE_INFO
//! \param    PMOS_SURFACE pSurface
//!           [in/out] Pointer to PMOS_SURFACE
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS RenderHal_GetSurfaceInfo(
    PMOS_INTERFACE              pOsInterface,
    PRENDERHAL_GET_SURFACE_INFO pInfo,
    PMOS_SURFACE                pSurface);

//!
//! \brief    Send Media States
//! \details  Send Media States
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \param    PRENDERHAL_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
//!           [in]    Pointer to GPGPU walker parameters
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendMediaStates(
    PRENDERHAL_INTERFACE      pRenderHal,
    PMOS_COMMAND_BUFFER       pCmdBuffer,
    PMHW_WALKER_PARAMS        pWalkerParams,
    PMHW_GPGPU_WALKER_PARAMS  pGpGpuWalkerParams);

//!
//! \brief    Issue command to write timestamp
//! \param    [in] pRenderHal
//! \param    [in] pCmdBuffer
//! \param    [in] bStartTime
//! \return   MOS_STATUS
//!
MOS_STATUS RenderHal_SendTimingData(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer,
    bool                         bStartTime);

// Constants defined in RenderHal interface
extern const MHW_PIPE_CONTROL_PARAMS      g_cRenderHal_InitPipeControlParams;
extern const MHW_VFE_PARAMS               g_cRenderHal_InitVfeParams;
extern const MHW_MEDIA_STATE_FLUSH_PARAM  g_cRenderHal_InitMediaStateFlushParams;
extern const RENDERHAL_KERNEL_PARAM       g_cRenderHal_InitKernelParams;

#endif // __RENDERHAL_H__
