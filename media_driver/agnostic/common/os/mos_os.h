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
//! \file     mos_os.h
//! \brief    Common interface and structure used in MOS OS
//!

#ifndef __MOS_OS_H__
#define __MOS_OS_H__

#include "mos_defs.h"
#include "media_skuwa_specific.h"
#include "mos_utilities.h"
#include "mos_os_hw.h"         //!< HW specific details that flow through OS pathes
#include "mos_os_cp_specific.h"         //!< CP specific OS functionality

//!
//! \brief OS specific includes and definitions
//!
#include "mos_os_specific.h"

#define MOS_NAL_UNIT_LENGTH                 4
#define MOS_NAL_UNIT_STARTCODE_LENGTH       3
#define MOS_MAX_PATH_LENGTH                 256

#define MOS_MAX_SEMAPHORE_COUNT             3
#define MOS_MAX_OBJECT_SIGNALED             32

#define MOS_INVALID_APPID                   0xFFFFFFFF

#define MOS_GPU_CONTEXT_CREATE_DEFAULT      1

#define MOS_VCS_ENGINE_USED(GpuContext) (              \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO)         || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO2)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO3)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO4)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VDBOX2_VIDEO)  || \
    ((GpuContext) == MOS_GPU_CONTEXT_VDBOX2_VIDEO2) || \
    ((GpuContext) == MOS_GPU_CONTEXT_VDBOX2_VIDEO3)    \
)

#define MOS_RCS_ENGINE_USED(GpuContext) (              \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER2)       || \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER3)       || \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER4)       || \
    ((GpuContext) == MOS_GPU_CONTEXT_COMPUTE)          \
)

#if MOS_MEDIASOLO_SUPPORTED
#define MOS_VECS_ENGINE_USED(GpuContext) (             \
    ((GpuContext) == MOS_GPU_CONTEXT_VEBOX)         || \
    ((GpuContext) == MOS_GPU_CONTEXT_VEBOX2))
#else
#define MOS_VECS_ENGINE_USED(GpuContext) (             \
    ((GpuContext) == MOS_GPU_CONTEXT_VEBOX))
#endif

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
#define MOS_COMMAND_BUFFER_OUT_FILE        "Command_buffer_output"
#define MOS_COMMAND_BUFFER_OUT_DIR         "Command_buffer_dumps"
#define MOS_COMMAND_BUFFER_RENDER_ENGINE   "CS"
#define MOS_COMMAND_BUFFER_VIDEO_ENGINE    "VCS"
#define MOS_COMMAND_BUFFER_VEBOX_ENGINE    "VECS"
#define MOS_COMMAND_BUFFER_PLATFORM_LEN    4
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#ifndef E_NOTIMPL
#define E_NOTIMPL       0x80004001L
#endif // E_NOTIMPL

#ifndef E_UNEXPECTED
#define E_UNEXPECTED    0x8000FFFFL
#endif // E_UNEXPECTED

//!
//! \brief Enum for OS component
//!
enum MOS_COMPONENT
{
    COMPONENT_UNKNOWN = 0,
    COMPONENT_LibVA,
    COMPONENT_EMULATION,
    COMPONENT_CM,
    COMPONENT_Encode,
    COMPONENT_Decode,
    COMPONENT_VPCommon,
    COMPONENT_VPreP,
    COMPONENT_CP,
    COMPONENT_MEMDECOMP,
};
C_ASSERT(COMPONENT_MEMDECOMP == 9); // When adding, update assert

//!
//! \brief Structure to OS sync parameters
//!
typedef struct _MOS_SYNC_PARAMS
{
    MOS_GPU_CONTEXT        GpuContext;          //!< GPU context you would like to signal on or wait in
    PMOS_RESOURCE          presSyncResource;    //!< Has 2 meanings:
    //!< 1) a resource that requires sync, like a destination surface
    //!< 2) a resource used by HW/OS to sync between engines, like for MI_SEMAPHORE_MBOX
    uint32_t                uiSemaphoreCount;
    uint32_t                uiSemaphoreValue;           //!< Tag value in the case of resource tagging
    uint32_t                uiSemaphoreOffset;          //!< Offset into the sync resource for value read/write
    int32_t                 bReadOnly;                  //!< Marks the resource as read or write for future waits
    int32_t                 bDisableDecodeSyncLock;     //!< Disable the lock function for decode.
    int32_t                 bDisableLockForTranscode;   //!< Disable the lock function for transcode perf.
} MOS_SYNC_PARAMS, *PMOS_SYNC_PARAMS;

#if (_DEBUG || _RELEASE_INTERNAL)
//!
//! \brief Enum for forcing VDBOX
//!
typedef enum _MOS_FORCE_VDBOX
{
    MOS_FORCE_VDBOX_NONE    = 0,
    MOS_FORCE_VDBOX_1       = 0x0001,
    MOS_FORCE_VDBOX_2       = 0x0002,
} MOS_FORCE_VDBOX;

#define MOS_FORCEVDBOX_VDBOXID_BITSNUM              4 //each VDBOX ID occupies 4 bits see defintion MOS_FORCE_VDBOX
#define MOS_FORCEVDBOX_MASK                         0xF

#define MOS_FORCEENGINE_MASK                         0xf
#define MOS_FORCEENGINE_ENGINEID_BITSNUM             4 //each VDBOX ID occupies 4 bits see defintion MOS_FORCE_VDBOX
#define MOS_INVALID_FORCEENGINE_VALUE                0xffffffff
#endif

typedef struct _MOS_COMMAND_BUFFER_ATTRIBUTES
{
    int32_t                     bTurboMode;
    int32_t                     bIsMdfLoad;
    int32_t                     bMediaPreemptionEnabled;
    uint32_t                    dwNumRequestedEUSlices;
    uint32_t                    dwNumRequestedSubSlices;
    uint32_t                    dwNumRequestedEUs;
    int32_t                     bValidPowerGatingRequest;
    int32_t                     bEnableMediaFrameTracking;
    uint32_t                    dwMediaFrameTrackingTag;
    uint32_t                    dwMediaFrameTrackingAddrOffset;
    MOS_RESOURCE                resMediaFrameTrackingSurface;
    int32_t                     bUmdSSEUEnable;
    void*                       pAttriExt;
} MOS_COMMAND_BUFFER_ATTRIBUTES, *PMOS_COMMAND_BUFFER_ATTRIBUTES;

//!
//! \brief Structure to command buffer
//!
typedef struct _MOS_COMMAND_BUFFER
{
    MOS_RESOURCE        OsResource;                 //!< OS Resource

    // Common fields
    uint32_t            *pCmdBase;                   //!< Base    address (CPU)
    uint32_t            *pCmdPtr;                    //!< Current address (CPU)
    int32_t             iOffset;                    //!< Current offset in bytes
    int32_t             iRemaining;                 //!< Remaining size
    int32_t             iTokenOffsetInCmdBuf;       //!< Pointer to (Un)Secure token's next field Offset
    int32_t             iCmdIndex;                  //!< command buffer's index

    MOS_COMMAND_BUFFER_ATTRIBUTES Attributes;       //!< Attributes for the command buffer to be provided to KMD at submission
} MOS_COMMAND_BUFFER;

//!
//! \brief Structure to Lock params
//!
typedef struct _MOS_LOCK_PARAMS
{
    union
    {
        struct
        {
            uint32_t ReadOnly            : 1;                                    //!< Lock only for reading.
            uint32_t WriteOnly           : 1;                                    //!< Lock only for writing.
            uint32_t TiledAsTiled        : 1;                                    //!< Means that you want to lock a tiled surface as tiled, not linear.
            uint32_t NoOverWrite         : 1;                                    //!< No Over write for async locks
            uint32_t NoDecompress        : 1;                                    //!< No decompression for memory compressed surface
            uint32_t Uncached            : 1;                                    //!< Use uncached lock
            uint32_t ForceCached         : 1;                                    //!< Prefer normal map to global GTT map(Uncached) if both can work
            uint32_t Reserved            : 25;                                   //!< Reserved for expansion.
        };
        uint32_t    Value;
    };
} MOS_LOCK_PARAMS, *PMOS_LOCK_PARAMS;

//!
//! \brief Structure to Resource Flags
//!
typedef struct _MOS_GFXRES_FLAGS
{
    int32_t         bNotLockable;                                               //!< [in] true: Resource will not be CPU accessible. false: Resource can be CPU accessed.
    int32_t         bOverlay;
    int32_t         bFlipChain;
    int32_t         bSVM;
} MOS_GFXRES_FLAGS, *PMOS_GFXRES_FLAGS;

//!
//! \brief Structure to Resource allocation parameters
//!
typedef struct _MOS_ALLOC_GFXRES_PARAMS
{
    MOS_GFXRES_TYPE     Type;                                                   //!< [in] Basic resource geometry
    MOS_GFXRES_FLAGS    Flags;                                                  //!< [in] Flags to describe attributes
    union
    {
        uint32_t        dwWidth;                                                //!< [in] Type == 2D || VOLUME, width in pixels.
        uint32_t        dwBytes;                                                //!< [in] Type == BUFFER, # of bytes
    };
    uint32_t            dwHeight;                                               //!< [in] Type == 2D || VOLUME, height in rows. Type == BUFFER, n/a
    uint32_t            dwDepth;                                                //!< [in] 0: Implies 2D resource. >=1: volume resource
    uint32_t            dwArraySize;                                            //!< [in] 0,1: 1 element. >1: N elements
    MOS_TILE_TYPE       TileType;                                               //!< [in] Defines the layout of a physical page. Optimal choice depends on usage model.
    MOS_FORMAT          Format;                                                 //!< [in] Pixel format
    void                *pSystemMemory;                                         //!< [in] Optional parameter. If non null, TileType must be set to linear.
    const char          *pBufName;                                              //!< [in] Optional parameter only used in Linux. A string indicates the buffer name and is used for debugging. nullptr is OK.
    int32_t             bIsCompressed;                                          //!< [in] Resource is compressed or not.
    MOS_RESOURCE_MMC_MODE   CompressionMode;                                        //!< [in] Compression mode.
    int32_t             bIsPersistent;                                          //!< [in] Optional parameter. Used to indicate that resource can not be evicted
    int32_t             bBypassMODImpl;
} MOS_ALLOC_GFXRES_PARAMS, *PMOS_ALLOC_GFXRES_PARAMS;

//!
//! \brief Structure to OS sync parameters
//!
typedef struct _MOS_PATCH_ENTRY_PARAMS
{
    PMOS_RESOURCE presResource;      //!< resource to be patched
    uint32_t      uiAllocationIndex;
    uint32_t      uiResourceOffset;  //!< resource offset
    uint32_t      uiPatchOffset;     //!< patch offset
    bool          bWrite;            //!< is write operation
    int32_t       bUpperBoundPatch;  //!< is upper bound patch
    MOS_HW_COMMAND              HwCommandType;     //!< hw cmd type
    uint32_t                    forceDwordOffset;  //!< force dword offset
    uint8_t                     *cmdBufBase; //!< cmd buffer base address
} MOS_PATCH_ENTRY_PARAMS, *PMOS_PATCH_ENTRY_PARAMS;

typedef struct _MOS_GPUCTX_CREATOPTIONS MOS_GPUCTX_CREATOPTIONS, *PMOS_GPUCTX_CREATOPTIONS;
struct _MOS_GPUCTX_CREATOPTIONS
{
    uint32_t  CmdBufferNumScale;
    _MOS_GPUCTX_CREATOPTIONS() : CmdBufferNumScale(MOS_GPU_CONTEXT_CREATE_DEFAULT) {}
    virtual ~_MOS_GPUCTX_CREATOPTIONS(){}
};

class OsContext;

//!
//! \brief Structure to Unified HAL OS resources
//!
typedef struct _MOS_INTERFACE
{
    // Saved OS context
    PMOS_CONTEXT                    pOsContext;
    MOS_GPU_CONTEXT                 CurrentGpuContextOrdinal;
    //!< An internal handle that indexes into the list of GPU Context object
    uint32_t                        CurrentGpuContextHandle;
    //!< A handle to the graphics context device that can be used to calls back
    //!< into the kernel subsystem
    HANDLE                          CurrentGpuContextRuntimeHandle;

    // OS dependent settings, flags, limits
    int32_t                         b64bit;
    int32_t                         bDeallocateOnExit;
    MOS_USER_FEATURE_INTERFACE      UserFeatureInterface;
    MosCpInterface                  *osCpInterface;

    int32_t                         bUsesCmdBufHeader;
    int32_t                         bUsesCmdBufHeaderInResize;
    int32_t                         bEnableKmdMediaFrameTracking;
    int32_t                         bNoParsingAssistanceInKmd;
    uint32_t                        dwCommandBufferReservedSpace;
    uint32_t                        dwNumNalUnitBytesIncluded;

    // GPU Reset Statistics
    uint32_t                        dwGPUResetCount;
    uint32_t                        dwGPUActiveBatch;
    uint32_t                        dwGPUPendingBatch;

    // Resource addressing
    int32_t                         bUsesPatchList;
    int32_t                         bUsesGfxAddress;
    int32_t                         bMapOnCreate;                           // For limited GPU VA resource can not be mapped during creation
    int32_t                         bInlineCodecStatusUpdate;               // check whether use inline codec status update or seperate BB

    // Component info
    MOS_COMPONENT                   Component;

    // Synchronization
    int32_t                         bTagEngineSync;
    int32_t                         bTagResourceSync;
    int32_t                         bOsEngineSync;
    int32_t                         bOsResourceSync;

    // Simulation (HAS) related
    int32_t                         bSimIsActive;                                   //!< Flag to indicate if HAS is enabled

    MOS_NULL_RENDERING_FLAGS        NullHWAccelerationEnable;                    //!< To indicate which components to enable Null HW support

    // for MODS Wrapper
    int32_t                         modulizedMosEnabled;
    int32_t                         modularizedGpuCtxEnabled;
    OsContext*                      osContextPtr;

    // used for media reset enabling/disabling in UMD
    // pls remove it after hw scheduling
    int32_t                         bMediaReset;

    bool                            umdMediaResetEnable;

#if MOS_MEDIASOLO_SUPPORTED
    // MediaSolo related
    int32_t                         bSoloInUse;                                   //!< Flag to indicate if MediaSolo is enabled
    void                            *pvSoloContext;                                //!< pointer to MediaSolo context
    uint32_t                        dwEnableMediaSoloFrameNum;                    //!< The frame number at which MediaSolo will be enabled, 0 is not valid.
#endif // MOS_MEDIASOLO_SUPPORTED

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    // Command buffer dump
    int32_t                         bDumpCommandBuffer;                                //!< Flag to indicate if Dump command buffer is enabled
    int32_t                         bDumpCommandBufferToFile;                          //!< Indicates that the command buffer should be dumped to a file
    int32_t                         bDumpCommandBufferAsMessages;                      //!< Indicates that the command buffer should be dumped via MOS normal messages
    char                            sPlatformName[MOS_COMMAND_BUFFER_PLATFORM_LEN];    //!< Platform name - maximum 4 bytes length
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_FORCE_VDBOX                 eForceVdbox;                                  //!< Force select Vdbox
    uint32_t                        dwForceTileYfYs;                              // force to allocate Yf (=1) or Ys (=2), remove after full validation
    int32_t                         bTriggerCodecHang;                            // trigger GPU HANG in codec
    int32_t                         bTriggerVPHang;                               //!< trigger GPU HANG in VP
#endif // (_DEBUG || _RELEASE_INTERNAL)

    MEMORY_OBJECT_CONTROL_STATE (* pfnCachePolicyGetMemoryObject) (
        MOS_HW_RESOURCE_DEF         Usage);

    MOS_STATUS (* pfnCreateGpuContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext,
        MOS_GPU_NODE                GpuNode,
        PMOS_GPUCTX_CREATOPTIONS    createOption);

    MOS_STATUS (* pfnDestroyGpuContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    MOS_STATUS (* pfnSetGpuContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    MOS_GPU_CONTEXT (* pfnGetGpuContext) (
        PMOS_INTERFACE              pOsInterface);

    MOS_STATUS (* pfnIsGpuContextValid) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    void (* pfnSetEncodeEncContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    void (* pfnSetEncodePakContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    void (* pfnSyncOnResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        MOS_GPU_CONTEXT             requestorGPUCtx,
        int32_t                     bWriteOperation);

    void (* pfnSyncOnOverlayResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        MOS_GPU_CONTEXT             requestorGPUCtx);

    void (* pfnSetResourceSyncTag) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_SYNC_PARAMS            pParams);

    MOS_STATUS (* pfnPerformOverlaySync) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_SYNC_PARAMS            pParams);

    MOS_STATUS (* pfnResourceSignal) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_SYNC_PARAMS            pParams);

    MOS_STATUS (* pfnResourceWait) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_SYNC_PARAMS            pParams);

    MOS_STATUS (* pfnEngineSignal) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_SYNC_PARAMS            pParams);

    MOS_STATUS (* pfnEngineWait) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_SYNC_PARAMS            pParams);

    MOS_STATUS (* pfnCreateSyncResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS (* pfnDestroySyncResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource);

    void (* pfnSyncGpuContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             busyGPUCtx,
        MOS_GPU_CONTEXT             requestorGPUCtx);

    void(* pfnSyncWith3DContext)(
        PMOS_INTERFACE              pOsInterface,
        PMOS_SYNC_PARAMS            pParams);

    MOS_STATUS (* pfnRegisterBBCompleteNotifyEvent) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    MOS_STATUS (* pfnWaitForBBCompleteNotifyEvent) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext,
        uint32_t                    uiTimeOut);

    void (* pfnDestroy) (
        PMOS_INTERFACE              pOsInterface,
        int32_t                     bDestroyVscVppDeviceTag);

    void (* pfnGetPlatform) (
        PMOS_INTERFACE              pOsInterface,
        PLATFORM                    *pPlatform);

    MEDIA_FEATURE_TABLE *(* pfnGetSkuTable) (
        PMOS_INTERFACE              pOsInterface);

    MEDIA_WA_TABLE *(* pfnGetWaTable) (
        PMOS_INTERFACE              pOsInterface);

    MEDIA_SYSTEM_INFO *(* pfnGetGtSystemInfo)(
        PMOS_INTERFACE     pOsInterface);

    void (* pfnResetOsStates) (
        PMOS_INTERFACE              pOsInterface);

    MOS_STATUS(* pfnInitializeMultiThreadingSyncTags) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        uint8_t                     ucRenderTargetIndex,
        PMOS_SEMAPHORE              *pCurFrmSem,
        PMOS_SEMAPHORE              *pRefFrmSem,
        PMOS_MUTEX                  *pFrmMutex);

    MOS_STATUS (* pfnMultiThreadingWaitCurrentFrame) (
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS (* pfnMultiThreadingPostCurrentFrame) (
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS (* pfnMultiThreadResourceSync) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS(*pfnSetHybridDecoderRunningFlag) (
        PMOS_INTERFACE              pOsInterface,
        int32_t                     bFlag);

    MOS_STATUS(*pfnGetHybridDecoderRunningFlag) (
        PMOS_INTERFACE              pOsInterface,
        int32_t                     *pFlag);

#if MOS_MESSAGES_ENABLED

    #define pfnAllocateResource(pOsInterface, pParams, pOsResource) \
       pfnAllocateResource(pOsInterface, pParams, __FUNCTION__, __FILE__, __LINE__, pOsResource)

    MOS_STATUS (* pfnAllocateResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_ALLOC_GFXRES_PARAMS    pParams,
        const char                  *functionName,
        const char                  *filename,
        int32_t                     line,
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS (* pfnDumpCommandBuffer) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    #define pfnFreeResource(pOsInterface, pResource) \
       pfnFreeResource(pOsInterface, __FUNCTION__, __FILE__, __LINE__, pResource)

    void (* pfnFreeResource) (
        PMOS_INTERFACE              pOsInterface,
        const char                  *functionName,
        const char                  *filename,
        int32_t                     line,
        PMOS_RESOURCE               pResource);
    
    #define pfnFreeResourceWithFlag(pOsInterface, pResource, uiFlag) \
       pfnFreeResourceWithFlag(pOsInterface, pResource, __FUNCTION__, __FILE__, __LINE__, uiFlag)

    void (* pfnFreeResourceWithFlag) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource,
        const char                  *functionName,
        const char                  *filename,
        int32_t                     line,
        uint32_t                    uiFlag);

#else

    MOS_STATUS (* pfnAllocateResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_ALLOC_GFXRES_PARAMS    pParams,
        PMOS_RESOURCE               pOsResource);

    void (* pfnFreeResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource);

    void (* pfnFreeResourceWithFlag) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource,
        uint32_t                    uiFlag);

#endif // MOS_MESSAGES_ENABLED

    MOS_STATUS (* pfnGetResourceInfo) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        PMOS_SURFACE                pDetails);

    MOS_STATUS (* pfnLockSyncRequest) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource,
        PMOS_LOCK_PARAMS            pFlags);

    void  *(* pfnLockResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource,
        PMOS_LOCK_PARAMS            pFlags);

    MOS_STATUS (* pfnUnlockResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource);

    MOS_STATUS (* pfnDecompResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource);

    MOS_STATUS(*pfnDoubleBufferCopyResource) (
        PMOS_INTERFACE        pOsInterface,
        PMOS_RESOURCE         pInputOsResource,
        PMOS_RESOURCE         pOutputOsResource);

    MOS_STATUS (* pfnFillResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource,
        uint32_t                    dwSize,
        uint8_t                     iValue);

    MOS_STATUS (* pfnRegisterResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource,
        int32_t                     bWrite,
        int32_t                     bWritebSetResourceSyncTag);

    void (* pfnResetResourceAllocationIndex) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource);

    int32_t (* pfnGetResourceAllocationIndex) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource);

    uint64_t (* pfnGetResourceGfxAddress) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource);

    MOS_STATUS (* pfnSetPatchEntry) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_PATCH_ENTRY_PARAMS     pParams);

#if MOS_MEDIASOLO_SUPPORTED
    MOS_STATUS (* pfnInitializeMediaSolo) (
        PMOS_INTERFACE              pOsInterface);
#endif // MOS_MEDIASOLO_SUPPORTED

    MOS_STATUS (* pfnWaitOnResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource);

    uint32_t (* pfnGetGpuStatusTag) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    void (* pfnIncrementGpuStatusTag) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    uint32_t (* pfnGetGpuStatusTagOffset) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    uint32_t (* pfnGetGpuStatusSyncTag) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    MOS_STATUS (* pfnVerifyCommandBufferSize) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    dwRequestedSize,
        uint32_t                    dwFlags);

    MOS_STATUS (* pfnResizeCommandBufferAndPatchList) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    dwRequestedCommandBufferSize,
        uint32_t                    dwRequestedPatchListSize,
        uint32_t                    dwFlags);

    MOS_STATUS (* pfnGetCommandBuffer) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        uint32_t                    dwFlags);

    MOS_STATUS (* pfnSetIndirectStateSize) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    uSize);

    MOS_STATUS (* pfnGetIndirectState) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    *puOffset,
        uint32_t                    *puSize);

    MOS_STATUS (* pfnGetIndirectStatePointer) (
        PMOS_INTERFACE              pOsInterface,
        uint8_t                     **pIndirectState);

    void (* pfnReturnCommandBuffer) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        uint32_t                    dwFlags);

    MOS_STATUS (* pfnSubmitCommandBuffer) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        int32_t                     bNullRendering);

    MOS_STATUS (* pfnWaitAndReleaseCmdBuffer) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    void (* pfnSleepMs) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    dwWaitMs);

    MOS_FORMAT (* pfnFmt_OsToMos) (
        MOS_OS_FORMAT               format);

    MOS_OS_FORMAT (* pfnFmt_MosToOs) (
        MOS_FORMAT                  format);

    GMM_RESOURCE_FORMAT (* pfnFmt_MosToGmm) (
        MOS_FORMAT                  format);

    void (* pfnSetPerfTag) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    PerfTag);

    uint32_t(* pfnGetPerfTag) (
        PMOS_INTERFACE              pOsInterface);

    void (* pfnResetPerfBufferID) (
        PMOS_INTERFACE              pOsInterface);

    void (* pfnIncPerfFrameID) (
        PMOS_INTERFACE              pOsInterface);

    void (* pfnIncPerfBufferID) (
        PMOS_INTERFACE              pOsInterface);

    void (* pfnSetPerfHybridKernelID) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    KernelID);

    int32_t (* pfnIsPerfTagSet) (
        PMOS_INTERFACE              pOsInterface);

    void (* pfnQueryPerformanceFrequency) (
        int64_t                     *pFrequency);

    void (* pfnQueryPerformanceCounter) (
        int64_t                     *pCount);

    MOS_STATUS (* pfnGetBitsPerPixel) (
        PMOS_INTERFACE              pOsInterface,
        MOS_FORMAT                  Format,
        uint32_t                    *piBpp);

    MOS_STATUS (*pfnLoadLibrary) (
        PMOS_INTERFACE              pOsInterface,
        const char                  *pFileName,
        PHMODULE                    phModule);

    MOS_STATUS (*pfnFreeLibrary) (
        HINSTANCE                   hInstance);

    void  *(*pfnGetProcAddress) (
        HINSTANCE                   hInstance,
        char                        *pModuleName);

    void (*pfnLogData)(
        char                        *pData);

    MOS_NULL_RENDERING_FLAGS  (* pfnGetNullHWRenderFlags) (
        PMOS_INTERFACE              pOsInterface);

    MOS_STATUS (* pfnSetCmdBufferDebugInfo) (
        PMOS_INTERFACE              pOsInterface,
        int32_t                     bSamplerState,
        int32_t                     bSurfaceState,
        uint32_t                    dwStateIndex,
        uint32_t                    dwType);

    uint32_t (* pfnGetCmdBufferDebugInfo) (
        PMOS_INTERFACE              pOsInterface,
        int32_t                     bSamplerState,
        int32_t                     bSurfaceState,
        uint32_t                    dwStateIndex);

    MOS_STATUS (*pfnGetGpuStatusBufferResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS (* pfnVerifyPatchListSize) (
        PMOS_INTERFACE              pOsInterface,
        uint32_t                    dwRequestedSize);

    MOS_STATUS (* pfnResetCommandBuffer) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnGetMemoryCompressionMode) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        PMOS_MEMCOMP_STATE          pResMmcMode);

    MOS_STATUS (* pfnSetMemoryCompressionMode) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        MOS_MEMCOMP_STATE           ResMmcMode);

    MOS_STATUS (* pfnSetMemoryCompressionHint) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        int32_t                     bHintOn);

    MOS_STATUS (* pfnCreateVideoNodeAssociation)(
        PMOS_INTERFACE              pOsInterface,
        int32_t                     bSetVideoNode,
        MOS_GPU_NODE                *pVideoNodeOrdinal);

    MOS_STATUS (* pfnDestroyVideoNodeAssociation)(
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_NODE                VideoNodeOrdinal);

    int32_t (*pfnSetCpuCacheability)(
        PMOS_INTERFACE              pOsInterface,
        PMOS_ALLOC_GFXRES_PARAMS    pParams);

    MOS_STATUS(*pfnSkipResourceSync)(
        PMOS_RESOURCE               pOsResource);

    bool(*pfnIsValidStreamID)(
        PMOS_RESOURCE               pOsResource);

    void(*pfnInvalidateStreamID)(
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS(*pfnSetStreamID)(
        PMOS_RESOURCE               pOsResource,
        PMOS_INTERFACE              pOsInterface,
        uint8_t                     streamID);

    uint8_t(*pfnGetStreamID)(
        PMOS_RESOURCE               pOsResource);

    int32_t (*pfnIsGPUHung)(
        PMOS_INTERFACE              pOsInterface);

    //!
    //! \brief    Get Aux Table base address
    //!
    //! \param    [in] pOsInterface
    //!             MOS_INTERFACE pointer
    //! \return   uint64_t 
    //!           return Aux Table base address
    //!
    uint64_t(*pfnGetAuxTableBaseAddr)(
        PMOS_INTERFACE              pOsInterface);

    //!
    //! \brief  Set slice count to shared memory and KMD
    //! \param  [in] pOsInterface
    //!         pointer to the requested slice count for current context
    //! \param  [out] pSliceCount
    //!         pointer to the ruling slice count shared by all contexts
    void (*pfnSetSliceCount)(
        PMOS_INTERFACE              pOsInterface,
        uint32_t *pSliceCount);

    //!
    //! \brief  Get resource index
    //! \param  [in] osResource
    //!         pointer to the resource
    //! \return uint32_t 
    //!         return resource index
    uint32_t(*pfnGetResourceIndex)(
        PMOS_RESOURCE           osResource);

    //!
    //! \brief    Get SetMarker enabled flag
    //! \details  Get SetMarker enabled flag from OsInterface
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \return   bool
    //!           SetMarker enabled flag
    //!
    bool (*pfnIsSetMarkerEnabled)(
        PMOS_INTERFACE              pOsInterface);

    //!
    //! \brief    Get SetMarker resource address
    //! \details  Get SetMarker resource address from OsInterface
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \return   PMOS_RESOURCE
    //!           SetMarker resource address
    //!
    PMOS_RESOURCE (*pfnGetMarkerResource)(
        PMOS_INTERFACE              pOsInterface);

    //!< os interface extension
    void                            *pOsExt;
} MOS_INTERFACE;

#ifdef __cplusplus
extern "C" {
#endif

//! \brief    Unified OS Initializes OS Interface
//! \details  OS Interface initilization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in/out] Pointer to OS Interface
//! \param    PMOS_CONTEXT pOsDriverContext
//!           [in] Pointer to Driver context
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_InitInterface(
    PMOS_INTERFACE      pOsInterface,
    PMOS_CONTEXT        pOsDriverContext,
    MOS_COMPONENT       component);

//! \brief    Unified OS add command to command buffer
//! \details  Offset returned is dword aligned but size requested can be byte aligned
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in/out] Pointer to Command Buffer
//! \param    void  *pCmd
//!           [in] Command Pointer
//! \param    uint32_t dwCmdSize
//!           [in] Size of command in bytes
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_AddCommand(
    PMOS_COMMAND_BUFFER pCmdBuffer,
    const void          *pCmd,
    uint32_t            dwCmdSize);

#if !EMUL
//!
//! \brief    Get memory object based on resource usage
//! \details  Get memory object based on resource usage
//! \param    MOS_HW_RESOURCE_DEF MosUsage
//!           [in] Current usage for resource
//! \return   MEMORY_OBJECT_CONTROL_STATE
//!           Populated memory object
//!
MEMORY_OBJECT_CONTROL_STATE Mos_CachePolicyGetMemoryObject(
    MOS_HW_RESOURCE_DEF         MosUsage);
#endif

#ifdef __cplusplus
}
#endif

#endif  // __MOS_OS_H__
