/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file      mos_os_specific.h
//! \brief     Common interface and structure used in MOS LINUX OS
//!

#ifndef __MOS_OS_SPECIFIC_H__
#define __MOS_OS_SPECIFIC_H__

#include "media_skuwa_specific.h"
#include "GmmLib.h"
#include "mos_resource_defs.h"
#include "mos_defs.h"
#include "mos_os_cp_interface_specific.h"
#ifdef ANDROID
#include <utils/Log.h>
#endif
#include "i915_drm.h"
#include "mos_bufmgr.h"
#include "xf86drm.h"

#include <vector>

typedef unsigned int MOS_OS_FORMAT;

class GraphicsResource;
class GraphicsResourceNext;
class AuxTableMgr;
class MosOcaInterface;
class GraphicsResourceNext;

////////////////////////////////////////////////////////////////////

typedef void* HINSTANCE;

#define MAKEFOURCC(ch0, ch1, ch2, ch3)  \
    ((uint32_t)(uint8_t)(ch0) | ((uint32_t)(uint8_t)(ch1) << 8) |  \
    ((uint32_t)(uint8_t)(ch2) << 16) | ((uint32_t)(uint8_t)(ch3) << 24 ))

#define GMM_LIBVA_LINUX 3

enum DdiSurfaceFormat
{
    DDI_FORMAT_UNKNOWN      = 0,
    DDI_FORMAT_A8B8G8R8     = 32,
    DDI_FORMAT_X8B8G8R8     = 33,
    DDI_FORMAT_A8R8G8B8     = 21,
    DDI_FORMAT_X8R8G8B8     = 22,
    DDI_FORMAT_R5G6B5       = 23,
    DDI_FORMAT_YUY2         = MAKEFOURCC('Y', 'U', 'Y', '2'),
    DDI_FORMAT_P8           = 41,
    DDI_FORMAT_A8P8         = 40,
    DDI_FORMAT_A8           = 28,
    DDI_FORMAT_L8           = 50,
    DDI_FORMAT_L16          = 81,
    DDI_FORMAT_A4L4         = 52,
    DDI_FORMAT_A8L8         = 51,
    DDI_FORMAT_R32F         = 114,
    DDI_FORMAT_V8U8         = 60,
    DDI_FORMAT_UYVY         = MAKEFOURCC('U', 'Y', 'V', 'Y'),
    DDI_FORMAT_NV12         = MAKEFOURCC('N', 'V', '1', '2'),
    DDI_FORMAT_A16B16G16R16 = 36,
    DDI_FORMAT_R32G32B32A32F = 115,
};

#define INDIRECT_HEAP_SIZE_UNITS    (1024)

/* copy from intcver.h  */
#ifndef BUILD_NUMBER
#define BUILD_NUMBER       9034
#endif

#define COMMAND_BUFFER_RESERVED_SPACE                      0x80

#define RtlEqualMemory(Destination,Source,Length) (!memcmp((Destination),(Source),(Length)))

////////////////////////////////////////////////////////////////////

//!
//! \brief Definitions specific to Linux
//!
#define COMMAND_BUFFER_SIZE                       32768

#define MAX_CMD_BUF_NUM                           30

#define MOS_LOCKFLAG_WRITEONLY                    OSKM_LOCKFLAG_WRITEONLY
#define MOS_LOCKFLAG_READONLY                     OSKM_LOCKFLAG_READONLY
#define MOS_LOCKFLAG_NOOVERWRITE                  OSKM_LOCKFLAG_NOOVERWRITE
#define MOS_LOCKFLAG_NO_SWIZZLE                   OSKM_LOCKFLAG_NO_SWIZZLE

#define MOS_DIR_SEPERATOR                         '/'

#define MOS_INVALID_ALLOC_INDEX                         -1
#define MOS_MAX_REGS                              128 //32
#ifdef ANDROID
#define MOS_STATUS_REPORT_DEFAULT                 0
#else
#define MOS_STATUS_REPORT_DEFAULT                 1
#endif

#define OSKM_LOCKFLAG_WRITEONLY                   0x00000001
#define OSKM_LOCKFLAG_READONLY                    0x00000002
#define OSKM_LOCKFLAG_NOOVERWRITE                 0x00000004
#define OSKM_LOCKFLAG_NO_SWIZZLE                  0x00000008

// should be defined in libdrm, this is a temporary solution to pass QuickBuild
#define I915_EXEC_VEBOX                  (4<<0)
#define I915_EXEC_VCS2                   (5<<0)

// I915_EXEC_BSD_* -- Attempt to provide backwards and forwards
// compatibility with versions of include/drm/i915_drm.h that do not
// have these definitions or that have them with the same values but
// different textual representations.  This will help avoid compiler
// warnings about macro redefinitions.

#if !defined I915_EXEC_BSD_MASK
#define I915_EXEC_BSD_MASK               (3<<13)
#endif
#if !defined I915_EXEC_BSD_DEFAULT
#define I915_EXEC_BSD_DEFAULT            (0<<13) /* default ping-pong mode */
#endif
#if !defined I915_EXEC_BSD_RING1
#define I915_EXEC_BSD_RING1              (1<<13)
#endif
#if !defined I915_EXEC_BSD_RING2
#define I915_EXEC_BSD_RING2              (2<<13)
#endif

#define I915_PARAM_HAS_BSD2               31
#define I915_EXEC_ENABLE_WATCHDOG_LINUX  (1<<18)

//!
//! \brief Forward declarations
//!
typedef struct _MOS_SPECIFIC_RESOURCE  MOS_RESOURCE, *PMOS_RESOURCE;
typedef struct _MOS_INTERFACE      *PMOS_INTERFACE;
typedef struct _MOS_COMMAND_BUFFER *PMOS_COMMAND_BUFFER;
typedef struct _MOS_LOCK_PARAMS    *PMOS_LOCK_PARAMS;

//!
//! \brief enum to video device operations 
//!
typedef enum _MOS_MEDIA_OPERATION
{
    MOS_MEDIA_OPERATION_NONE    = 0,
    MOS_MEDIA_OPERATION_DECODE,
    MOS_MEDIA_OPERATION_ENCODE,
    MOS_MEDIA_OPERATION_MAX
} MOS_MEDIA_OPERATION, *PMOS_MEDIA_OPERATION;

//!
//! \brief ENum to GPU nodes
//!
typedef enum _MOS_GPU_NODE
{
    MOS_GPU_NODE_3D      = I915_EXEC_RENDER,
    MOS_GPU_NODE_COMPUTE = (6<<0), //To change to compute CS later when linux define the name
    MOS_GPU_NODE_VE      = I915_EXEC_VEBOX,
    MOS_GPU_NODE_VIDEO   = I915_EXEC_BSD,
    MOS_GPU_NODE_VIDEO2  = I915_EXEC_VCS2,
    MOS_GPU_NODE_BLT     = I915_EXEC_BLT,
    MOS_GPU_NODE_MAX     = 7,//GFX_MAX(I915_EXEC_RENDER, I915_EXEC_VEBOX, I915_EXEC_BSD, I915_EXEC_VCS2, I915_EXEC_BLT) + 1
} MOS_GPU_NODE, *PMOS_GPU_NODE;

//!
//! \brief Inline function to get GPU node
//!
static inline MOS_GPU_NODE OSKMGetGpuNode(MOS_GPU_CONTEXT uiGpuContext)
{
    switch (uiGpuContext)
    {
        case MOS_GPU_CONTEXT_RENDER:
        case MOS_GPU_CONTEXT_RENDER2:
        case MOS_GPU_CONTEXT_RENDER3:
        case MOS_GPU_CONTEXT_RENDER4:
        case MOS_GPU_CONTEXT_COMPUTE: //change this context mapping to Compute Node instead of 3D node after the node name is defined in linux.
            return  MOS_GPU_NODE_3D;
            break;
        case MOS_GPU_CONTEXT_VEBOX:
            return  MOS_GPU_NODE_VE;
            break;
        case MOS_GPU_CONTEXT_VIDEO:
        case MOS_GPU_CONTEXT_VIDEO2:
        case MOS_GPU_CONTEXT_VIDEO3:
        case MOS_GPU_CONTEXT_VIDEO4:
        case MOS_GPU_CONTEXT_VIDEO5:
        case MOS_GPU_CONTEXT_VIDEO6:
        case MOS_GPU_CONTEXT_VIDEO7:
            return MOS_GPU_NODE_VIDEO;
            break;
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO2:
        case MOS_GPU_CONTEXT_VDBOX2_VIDEO3:
            return MOS_GPU_NODE_VIDEO2;
            break;
        case MOS_GPU_CONTEXT_BLT:
            return MOS_GPU_NODE_BLT;
            break;  
        default:
            return MOS_GPU_NODE_MAX ;
            break;
    }
}

//!
//! \brief enum to Intel bo map operations 
//!
typedef enum _MOS_MMAP_OPERATION
{
    MOS_MMAP_OPERATION_NONE    = 0,
    MOS_MMAP_OPERATION_MMAP,
    MOS_MMAP_OPERATION_MMAP_GTT,
    MOS_MMAP_OPERATION_MMAP_WC
} MOS_MMAP_OPERATION, *PMOS_MMAP_OPERATION;

//!
//! \brief Structure to Linux resource
//!
struct _MOS_SPECIFIC_RESOURCE
{
    int32_t             iWidth;
    int32_t             iHeight;
    int32_t             iSize;
    int32_t             iPitch;
    int32_t             iDepth;             //!< for 3D surface
    MOS_FORMAT          Format;
    int32_t             iCount;
    int32_t             iAllocationIndex[MOS_GPU_CONTEXT_MAX];
    uint32_t            dwGfxAddress;
    uint8_t             *pData;
    const char          *bufname;
    uint32_t            isTiled;
    MOS_TILE_TYPE       TileType;
    uint32_t            bMapped;
    MOS_LINUX_BO        *bo;
    uint32_t            name;
    GMM_RESOURCE_INFO   *pGmmResInfo;        //!< GMM resource descriptor
    MOS_MMAP_OPERATION  MmapOperation;
    uint8_t             *pSystemShadow;
    bool                b16UsrPtrMode;      //!< indicate source info comes from app.
    MOS_PLANE_OFFSET    YPlaneOffset;       //!< Y surface plane offset
    MOS_PLANE_OFFSET    UPlaneOffset;       //!< U surface plane offset
    MOS_PLANE_OFFSET    VPlaneOffset;       //!< V surface plane offset

    //!< to sync render target for multi-threading decoding mode
    struct
    {
        int32_t         bSemInitialized;
        PMOS_SEMAPHORE  *ppCurrentFrameSemaphore;   //!< Semaphore queue for hybrid decoding multi-threading case
        PMOS_SEMAPHORE  *ppReferenceFrameSemaphore; //!< Semaphore queue for hybrid decoding multi-threading case; post when a surface is not used as reference frame
    };

#if MOS_MEDIASOLO_SUPPORTED
    //!< these fields are only used while MediaSolo is enabled(bSoloInUse of OS_Interface is true).
    uint32_t            dwOffset;
    FILE*               pFile;
    char                *pcFilePath;
    int32_t             bManualSwizzlingInUse;
    // used for AubCapture mode
    int32_t                 bAubGttUpdate;
    int32_t                 bAubMemUpdate;
    int32_t                 bInterestedRes;
    int32_t                 bPermaLocked;
#endif // MOS_MEDIASOLO_SUPPORTED

    // This is used by MDF when a wrapper/virtual MOS Resource is used to set surface state for a given VA, not necessary from start, in an actual MOS resource
    uint64_t                user_provided_va;
    // for MODS Wrapper
    GraphicsResource*       pGfxResource;
    GraphicsResourceNext*   pGfxResourceNext;
    bool                    bConvertedFromDDIResource;

};

//!
//! \brief Structure to MOS_SURFACE
//!
struct MOS_SURFACE
{
    MOS_RESOURCE        OsResource;                                             //Surface Resource

    uint32_t            dwArraySlice;                                           //!< [in]
    uint32_t            dwMipSlice;                                             //!< [in]
    MOS_S3D_CHANNEL     S3dChannel;                                             //!< [in]

    MOS_GFXRES_TYPE     Type;                                                   //!< [out] Basic resource geometry
    int32_t             bOverlay;
    int32_t             bFlipChain;

    uint32_t            dwWidth;                                                //!< [out] Type == 2D || VOLUME, width in pixels.
    uint32_t            dwHeight;                                               //!< [out] Type == 2D || VOLUME, height in rows. Type == BUFFER, n/a
    uint32_t            dwSize;                                                 //!< [out] Type == 2D || VOLUME, the size of surface
    uint32_t            dwDepth;                                                //!< [out] 0: Implies 2D resource. >=1: volume resource
    uint32_t            dwArraySize;                                            //!< [out] 0,1: 1 element. >1: N elements
    uint32_t            dwLockPitch;                                            //!< [out] pitch in bytes used for locking
    uint32_t            dwPitch;                                                //!< [out] < RenderPitch > pitch in bytes used for programming HW
    uint32_t            dwSlicePitch;                                           //!< [out] Type == VOLUME, byte offset to next slice. Type != VOLUME, n/a
    uint32_t            dwQPitch;                                               //!< [out] QPitch - distance in rows between R-Planes used for programming HW
    MOS_TILE_TYPE       TileType;                                               //!< [out] Defines the layout of a physical page. Optimal choice depends on usage model.
    MOS_FORMAT          Format;                                                 //!< [out] Pixel format
    int32_t             bArraySpacing;                                          //!< [out] Array spacing
    int32_t             bCompressible;                                          //!< [out] Memory compression

    uint32_t            dwOffset;                                               // Surface Offset (Y/Base)
    MOS_PLANE_OFFSET    YPlaneOffset;                                           // Y surface plane offset
    MOS_PLANE_OFFSET    UPlaneOffset;                                           // U surface plane offset
    MOS_PLANE_OFFSET    VPlaneOffset;                                           // V surface plane

    union
    {
        struct
        {
            MOS_RESOURCE_OFFSETS Y;
            MOS_RESOURCE_OFFSETS U;
            MOS_RESOURCE_OFFSETS V;
        } YUV;                                                                  //!< [out] Valid for YUV & planar RGB formats. Invalid for RGB formats.

        MOS_RESOURCE_OFFSETS RGB;                                               //!< [out] Valid non planar RGB formats. Invalid for YUV and planar RGB formats.
    } RenderOffset;                                                             //!< [out] Offsets request by input parameters. Used to program HW.

    union
    {
        struct
        {
            uint32_t Y;
            uint32_t U;
            uint32_t V;
        } YUV;                                                                   //!< [out] Valid for YUV & planar RGB formats. Invalid for RGB formats.

        uint32_t RGB;                                                             //!< [out] Valid non planar RGB formats. Invalid for YUV and planar RGB formats.
    } LockOffset;                                                                //!< [out] Offset in bytes used for locking

    // Surface compression mode, enable flags
    int32_t                 bIsCompressed;                                       //!< [out] Memory compression flag
    MOS_RESOURCE_MMC_MODE   CompressionMode;                                     //!< [out] Memory compression mode
    uint32_t                CompressionFormat;                                   //!< [out] Memory compression format
    // deprecated: not to use MmcState
    MOS_MEMCOMP_STATE       MmcState;                                            // Memory compression state
};
typedef MOS_SURFACE *PMOS_SURFACE;

//!
//! \brief Structure to MOS_BUFFER
//!
struct MOS_BUFFER
{
    MOS_RESOURCE    OsResource; //!< Buffer resource
    uint32_t        size;       //!< Buffer size
    const char*     name;           //!< Buffer name
    bool            initOnAllocate; //!< Flag to indicate whether initialize when allocate
    uint8_t         initValue;      //!< Initialize value when initOnAllocate is set
    bool            bPersistent;    //!< Persistent flag
};
typedef MOS_BUFFER *PMOS_BUFFER;

//!
//! \brief Structure to patch location list
//!
typedef struct _PATCHLOCATIONLIST
{
    uint32_t                    AllocationIndex;
    uint32_t                    AllocationOffset;
    uint32_t                    PatchOffset;
    uint32_t                    cpCmdProps;
    int32_t                     uiRelocFlag;
    uint32_t                    uiWriteOperation;
    MOS_LINUX_BO                *cmdBo;
} PATCHLOCATIONLIST, *PPATCHLOCATIONLIST;

//#define PATCHLOCATIONLIST_SIZE 25
#define CODECHAL_MAX_REGS  128
#define PATCHLOCATIONLIST_SIZE CODECHAL_MAX_REGS

//!
//! \brief Structure to Allocation List
//!
typedef struct _ALLOCATION_LIST
{
    HANDLE      hAllocation;
    uint32_t    WriteOperation;
} ALLOCATION_LIST, *PALLOCATION_LIST;

//#define ALLOCATIONLIST_SIZE MOS_MAX_REGS
#define ALLOCATIONLIST_SIZE CODECHAL_MAX_REGS  //!< use the large value

//!
//! \brief Structure to command buffer
//!
typedef struct _COMMAND_BUFFER
{
    // Double linked list
    struct _COMMAND_BUFFER     *pNext;
    struct _COMMAND_BUFFER     *pPrev;

    int64_t             *pSyncTag;
    int64_t             qSyncTag;

    // Status
    int32_t             bActive;        //!< Active / Inactive flag
    int32_t             bRunning;       //!< CB is running in Gfx Device
    LARGE_INTEGER       TimeStart;      //!< Start timestamp
    LARGE_INTEGER       TimeEnd;        //!< End timestamp

    // Buffer information
    uint8_t             *pCmdBase;       //!< Pointer to buffer data
    uint8_t             *pCmdCurrent;    //!< Current pointer
    int32_t             iSize;          //!< Buffer Size
    int32_t             iCurrent;       //!< Current offset
    int32_t             iRemaining;     //!< Remaining
} COMMAND_BUFFER, *PCOMMAND_BUFFER;

typedef struct _MOS_GPU_STATUS_DATA
{
    uint32_t    GPUTag;
    uint32_t    ReservedForGPUTag;      //!< Reserved for gpu tag uint64_t write back by gpu but we only use the low uint32_t above.
    uint32_t    Reserved[6];            //!< Padding to 32 byte-aligned for future use.
} MOS_GPU_STATUS_DATA;

//!
//! \brief Structure to Gpu context
//!
typedef struct _CODECHAL_OS_GPU_CONTEXT
{
    volatile int32_t            bCBFlushed; //!< if CB not flushed, re-use
    PMOS_COMMAND_BUFFER         pCB;
    uint32_t                    uiCommandBufferSize;

    // Allcoation List
    ALLOCATION_LIST             *pAllocationList;
    uint32_t                    uiNumAllocations;
    uint32_t                    uiMaxPatchLocationsize;

    // Patch List
    PATCHLOCATIONLIST           *pPatchLocationList;
    uint32_t                    uiCurrentNumPatchLocations;
    uint32_t                    uiMaxNumAllocations;

    // OS command buffer
    PCOMMAND_BUFFER             pStartCB;
    PCOMMAND_BUFFER             pCurrentCB;

    // Resource registrations
    uint32_t                    uiResCount;                    //!< # of resources registered
    int32_t                     iResIndex[CODECHAL_MAX_REGS];  //!< Resource indices
    PMOS_RESOURCE                pResources;                   //!< Pointer to resources list
    int32_t                     *pbWriteMode;                  //!< Write mode

    // GPU Status
    uint32_t                    uiGPUStatusTag;
} MOS_OS_GPU_CONTEXT, *PMOS_OS_GPU_CONTEXT;

//!
//! \brief Structure to buffer pool
//!
typedef struct _CMD_BUFFER_BO_POOL
{
    int32_t             iFetch;
    MOS_LINUX_BO        *pCmd_bo[MAX_CMD_BUF_NUM];
}CMD_BUFFER_BO_POOL;

struct MOS_CONTEXT_OFFSET
{
    MOS_LINUX_CONTEXT *intel_context;
    MOS_LINUX_BO      *target_bo;
    uint64_t          offset64;
};

// APO related
#define FUTURE_PLATFORM_MOS_APO   1234
void SetupApoMosSwitch(PLATFORM *platform);

enum OS_SPECIFIC_RESOURCE_TYPE
{
    OS_SPECIFIC_RESOURCE_INVALID = 0,
    OS_SPECIFIC_RESOURCE_SURFACE = 1,
    OS_SPECIFIC_RESOURCE_BUFFER = 2,
    OS_SPECIFIC_RESOURCE_MAX
};

class OsContextNext;
typedef OsContextNext    OsDeviceContext;
typedef OsDeviceContext *MOS_DEVICE_HANDLE;

typedef struct _MOS_OS_CONTEXT MOS_CONTEXT, *PMOS_CONTEXT, MOS_OS_CONTEXT, *PMOS_OS_CONTEXT, MOS_DRIVER_CONTEXT,*PMOS_DRIVER_CONTEXT;
//!
//! \brief Structure to OS context
//!
struct _MOS_OS_CONTEXT
{
    // Context must be freed by os emul layer
    int32_t             bFreeContext;

    uint32_t            uIndirectStateSize;

    MOS_OS_GPU_CONTEXT  OsGpuContext[MOS_GPU_CONTEXT_MAX];

    // Buffer rendering
    LARGE_INTEGER       Frequency;                //!< Frequency
    LARGE_INTEGER       LastCB;                   //!< End time for last CB

    CMD_BUFFER_BO_POOL  CmdBufferPool;

    // Emulated platform, sku, wa tables
    PLATFORM            platform;
    MEDIA_FEATURE_TABLE   SkuTable;
    MEDIA_WA_TABLE            WaTable;
    MEDIA_SYSTEM_INFO      gtSystemInfo;

    // Controlled OS resources (for analysis)
    MOS_BUFMGR       *bufmgr;
    MOS_LINUX_CONTEXT   *intel_context;
    int32_t             submit_fence;
    uint32_t            uEnablePerfTag;           //!< 0: Do not pass PerfTag to KMD, perf data collection disabled;
                                                  //!< 1: Pass PerfTag to MVP driver, perf data collection enabled;
                                                  //!< 2: Pass PerfTag to DAPC driver, perf data collection enabled;
    int32_t             bDisableKmdWatchdog;      //!< 0: Do not disable kmd watchdog, that is to say, pass I915_EXEC_ENABLE_WATCHDOG flag to KMD;
                                                  //!< 1: Disable kmd watchdog, that is to say, DO NOT pass I915_EXEC_ENABLE_WATCHDOG flag to KMD;
    PERF_DATA           *pPerfData;               //!< Add Perf Data for KMD to capture perf tag

    int32_t             bHybridDecoderRunningFlag;      //!< Flag to indicate if hybrid decoder is running

    int                 iDeviceId;
    int                 wRevision;
    int32_t             bIsAtomSOC;
    int                 fd;                     //!< handle for /dev/dri/card0

    int32_t             bUse64BitRelocs;
    bool                bUseSwSwizzling;
    bool                bTileYFlag;

    void                **ppMediaMemDecompState; //!<Media memory decompression data structure

    // For modulized GPU context
    void*               m_gpuContextMgr;
    void*               m_cmdBufMgr;
    MOS_DEVICE_HANDLE   m_osDeviceContext = nullptr;

    //For 2VD box
    int32_t             bKMDHasVCS2;
    bool                bPerCmdBufferBalancing;
    int32_t             semid;
    int32_t             shmid;
    void                *pShm;

    uint32_t            *pTranscryptedKernels;     //!< The cached version for current set of transcrypted and authenticated kernels
    uint32_t            uiTranscryptedKernelsSize; //!< Size in bytes of the cached version of transcrypted and authenticated kernels
    void                *pLibdrmHandle;

    GMM_CLIENT_CONTEXT  *pGmmClientContext;   //UMD specific ClientContext object in GMM
    GmmExportEntries    GmmFuncs;
    AuxTableMgr         *m_auxTableMgr;
   
    // GPU Status Buffer
    PMOS_RESOURCE   pGPUStatusBuffer;

    std::vector< struct MOS_CONTEXT_OFFSET> contextOffsetList;

    // Media memory decompression function
    void (* pfnMemoryDecompress)(
        PMOS_CONTEXT                pOsContext,
        PMOS_RESOURCE               pOsResource);

    // Os Context interface functions
    void (* pfnDestroy)(
        struct _MOS_OS_CONTEXT      *pOsContext,
        int32_t                     MODSEnabled,
        int32_t                     MODSForGpuContext);

    int32_t (* pfnRefresh)(
        struct _MOS_OS_CONTEXT      *pOsContext);

    int32_t (* pfnGetCommandBuffer)(
        struct _MOS_OS_CONTEXT      *pOsContext,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        int32_t                     iSize);

    void (* pfnReturnCommandBuffer)(
        struct _MOS_OS_CONTEXT      *pOsContext,
        MOS_GPU_CONTEXT             GpuContext,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    int32_t (* pfnFlushCommandBuffer)(
        struct _MOS_OS_CONTEXT      *pOsContext,
        MOS_GPU_CONTEXT             GpuContext);

    MOS_STATUS (* pfnInsertCmdBufferToPool)(
        struct _MOS_OS_CONTEXT      *pOsContext,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnWaitAndReleaseCmdBuffer)(
        struct _MOS_OS_CONTEXT      *pOsContext,
        int32_t                     index);

    uint32_t (* GetDmaBufID ) (
        struct _MOS_OS_CONTEXT      *pOsContext);

    void (* SetDmaBufID ) (
        struct _MOS_OS_CONTEXT      *pOsContext,
        uint32_t                    dwDmaBufID);

    void (* SetPerfHybridKernelID ) (
        struct _MOS_OS_CONTEXT      *pOsContext,
        uint32_t                    KernelID);

    uint32_t (* pfnGetGpuCtxBufferTag)(
        PMOS_CONTEXT               pOsContext,
        MOS_GPU_CONTEXT            GpuContext);

    void (* pfnIncGpuCtxBufferTag)(
        PMOS_CONTEXT               pOsContext,
        MOS_GPU_CONTEXT            GpuContext);

    uint32_t (* GetGPUTag)(
        PMOS_INTERFACE             pOsInterface,
        MOS_GPU_CONTEXT            GpuContext);

    GMM_CLIENT_CONTEXT* (* GetGmmClientContext)(
        PMOS_CONTEXT               pOsContext);

    MosOcaInterface* (*GetOcaInterface)();
};

//!
//! \brief Structure to VDBOX Workload Type
//!
typedef struct _VDBOX_WORKLOAD
{
    uint32_t         uiVDBoxCount[2];    // VDBox workloads
    uint32_t         uiRingIndex;        // ping-pong when vdbox1 count equals vdbox2 count
}VDBOX_WORKLOAD, *PVDBOX_WORKLOAD;

//!
//! \brief Structure to Android Resource Type
//!

//!
//! \brief Structure to Android Resource structure
//!
typedef struct _MOS_OS_ALLOCATION
{
    PMOS_RESOURCE           pOsResource;                                        //!< Resource Info
    int32_t                 bWriteMode;                                         //!< Read/Write Mode
} MOS_OS_ALLOCATION, *PMOS_OS_ALLOCATION;

//!
//! \brief Structure to Buffer structure
//!
typedef struct _MOS_SPECIFIC_BUFFER
{
    uint32_t dwHandle;                                                           //!< Buffer Handle
    int32_t iAllocationIndex;                                                   //!< Allocation Index
} MOS_SPECIFIC_BUFFER_ANDROID, *PMOS_SPECIFIC_BUFFER;

#ifdef __cplusplus
extern "C" {
#endif

//!
//! \brief    Check if OS resource is nullptr
//! \details  Check if OS resource is nullptr
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \return   int32_t
//!           Return true if nullptr, otherwise false
//!
int32_t Mos_ResourceIsNull(
    PMOS_RESOURCE pOsResource);

//!
//! \brief    Get Buffer Type
//! \details  Returns the type of buffer, 1D, 2D or volume
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \return   GFX resource Type
//!
MOS_GFXRES_TYPE GetResType(
    PMOS_RESOURCE pOsResource);

//!
//! \brief    OS reset resource
//! \details  Resets the OS resource
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to OS Resource
//! \return   void
//!           Return NONE
//!
void Mos_ResetResource(
    PMOS_RESOURCE pOsResource);

//!
//! \brief    Convert to MOS tile type
//! \details  Convert from Linux to MOS tile type
//! \param    uint32_t type
//!           [in] tile type
//! \return   MOS_TILE_TYPE
//!           Return MOS tile type
//!
MOS_TILE_TYPE LinuxToMosTileType(
    uint32_t type);

//!
//! \brief    Linux OS initilization
//! \details  Linux OS initilization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    PMOS_CONTEXT pOsDriverContext
//!           [in] Pointer to OS Driver context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_InitInterface(
    PMOS_INTERFACE     pOsInterface,
    PMOS_CONTEXT       pOsDriverContext);

//!
//! \brief    Lock resource
//! \details  Lock allocated resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \param    PMOS_LOCK_PARAMS pLockFlags
//!           [in] Lock Flags - MOS_LOCKFLAG_* flags
//! \return   void *
//!
void  *Mos_Specific_LockResource(
    PMOS_INTERFACE        pOsInterface,
    PMOS_RESOURCE         pOsResource,
    PMOS_LOCK_PARAMS      pLockFlags);

//!
//! \brief    Destroys OS specific allocations
//! \details  Destroys OS specific allocations including destroying OS context
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \param    int32_t bDestroyVscVppDeviceTag
//!           [in] Destroy VscVppDeviceTagId Flag, no use in Linux
//! \return   void
//!
void Mos_Specific_Destroy(
    PMOS_INTERFACE     pOsInterface,
    int32_t            bDestroyVscVppDeviceTag);

//!
//! \brief    Resets OS States
//! \details  Resets OS States for linux
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface
//! \return   void
//!
void Mos_Specific_ResetOsStates(
    PMOS_INTERFACE pOsInterface);

//!
//! \brief    Resizes the buffer to be used for rendering GPU commands
//! \details  Resizes the buffer to be used for rendering GPU commands
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \param    uint32_t dwRequestedCommandBufferSize
//!           [in] Requested command buffer size
//! \param    uint32_t dwRequestedPatchListSize
//!           [in] Requested patch list size
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
//!
MOS_STATUS Mos_Specific_ResizeCommandBufferAndPatchList(
    PMOS_INTERFACE          pOsInterface,
    uint32_t                dwRequestedCommandBufferSize,
    uint32_t                dwRequestedPatchListSize,
    uint32_t                dwFlags);

//!
//! \brief    Unlock resource
//! \details  Unlock the locked resource
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_UnlockResource(
    PMOS_INTERFACE        pOsInterface,
    PMOS_RESOURCE         pOsResource);

//!
//! \brief    Get Resource Information
//! \details  Linux get resource info
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS interface structure
//! \param    PMOS_RESOURCE pOsResource
//!           [in] Pointer to input OS resource
//! \param    PMOS_SURFACE pResDetails
//!           [out] Pointer to output resource information details
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_Specific_GetResourceInfo(
    PMOS_INTERFACE              pOsInterface,
    PMOS_RESOURCE               pOsResource,
    PMOS_SURFACE                pResDetails);

//!
//! \brief    Get resource index
//! \details  Get resource index of MOS_RESOURCE
//! \param    PMOS_RESOURCE osResource
//!           [in] Pointer to OS resource
//! \return   uint32_t
//!           Resource index
//!
uint32_t Mos_Specific_GetResourceIndex(
    PMOS_RESOURCE               osResource);

uint32_t Mos_Specific_GetResourcePitch(
    PMOS_RESOURCE               pOsResource);

void Mos_Specific_SetResourceWidth(
    PMOS_RESOURCE               pOsResource,
    uint32_t                    dwWidth);

void Mos_Specific_SetResourceFormat(
    PMOS_RESOURCE               pOsResource,
    MOS_FORMAT                  mosFormat);

//!
//! \brief    Get SetMarker enabled flag
//! \details  Get SetMarker enabled flag from OsInterface
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   bool
//!           SetMarker enabled flag
//!
bool Mos_Specific_IsSetMarkerEnabled(
    PMOS_INTERFACE         pOsInterface);

//!
//! \brief    Get SetMarker resource address
//! \details  Get SetMarker resource address from OsInterface
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   PMOS_RESOURCE
//!           SetMarker resource address
//!
PMOS_RESOURCE Mos_Specific_GetMarkerResource(
    PMOS_INTERFACE         pOsInterface);

//!
//! \brief    Get TimeStamp frequency base
//! \details  Get TimeStamp frequency base from OsInterface
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   uint32_t
//!           time stamp frequency base
//!
uint32_t Mos_Specific_GetTsFrequency(
    PMOS_INTERFACE         pOsInterface);

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_LINUX_BO * Mos_GetNopCommandBuffer_Linux(
    PMOS_INTERFACE        pOsInterface);

MOS_LINUX_BO * Mos_GetBadCommandBuffer_Linux(
    PMOS_INTERFACE        pOsInterface);
#endif

#ifdef __cplusplus
}
#endif

#endif // __MOS_OS_SPECIFIC_H__
