/*
* Copyright (c) 2009-2024, Intel Corporation
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
#include "mos_utilities.h"
#include "media_skuwa_specific.h"
#include "mos_util_debug.h"
#include "mos_os_hw.h"         //!< HW specific details that flow through OS pathes
#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
#include "work_queue_mngr.h"
#include <FirmwareManager/GpuFW/GucModule/commoninc/KmGucClientInterface.h>
#endif
#endif

#include "media_user_setting_specific.h"
#include "null_hardware.h"
//!
//! \brief OS specific includes and definitions
//!
#include "mos_os_specific.h"
#include "mos_os_virtualengine_specific.h"

#include "mos_oca_interface.h"
#include "mos_cache_manager.h"

#define MOS_NAL_UNIT_LENGTH                 4
#define MOS_NAL_UNIT_STARTCODE_LENGTH       3
#define MOS_MAX_PATH_LENGTH                 256

#define MOS_MAX_SEMAPHORE_COUNT             3
#define MOS_MAX_OBJECT_SIGNALED             32

#define MOS_INVALID_APPID                   0xFFFFFFFF

#define MOS_GPU_CONTEXT_CREATE_DEFAULT      1
#define MOS_GPU_CONTEXT_CREATE_CM_DEFAULT   15

#define MOS_VCS_ENGINE_USED(GpuContext) (              \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO)         || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO2)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO3)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO4)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VDBOX2_VIDEO)  || \
    ((GpuContext) == MOS_GPU_CONTEXT_VDBOX2_VIDEO2) || \
    ((GpuContext) == MOS_GPU_CONTEXT_VDBOX2_VIDEO3) || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO5)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO6)        || \
    ((GpuContext) == MOS_GPU_CONTEXT_VIDEO7)           \
)

#define MOS_RCS_ENGINE_USED(GpuContext) (                 \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER)           || \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER2)          || \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER3)          || \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER4)          || \
    ((GpuContext) == MOS_GPU_CONTEXT_COMPUTE)          || \
    ((GpuContext) == MOS_GPU_CONTEXT_CM_COMPUTE)       || \
    ((GpuContext) == MOS_GPU_CONTEXT_COMPUTE_RA)       || \
    ((GpuContext) == MOS_GPU_CONTEXT_RENDER_RA)           \
)

#define MOS_BCS_ENGINE_USED(GpuContext) (             \
    ((GpuContext) == MOS_GPU_CONTEXT_BLT))

#if MOS_MEDIASOLO_SUPPORTED
#define MOS_VECS_ENGINE_USED(GpuContext) (             \
    ((GpuContext) == MOS_GPU_CONTEXT_VEBOX)         || \
    ((GpuContext) == MOS_GPU_CONTEXT_VEBOX2))
#else
#define MOS_VECS_ENGINE_USED(GpuContext) (             \
    ((GpuContext) == MOS_GPU_CONTEXT_VEBOX))
#endif

#define MOS_TEECS_ENGINE_USED(GpuContext) (             \
    ((GpuContext) == MOS_GPU_CONTEXT_TEE))

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
#define MOS_COMMAND_BUFFER_OUT_FILE        "Command_buffer_output"
#define MOS_COMMAND_BUFFER_OUT_DIR         "Command_buffer_dumps"
#define MOS_COMMAND_BUFFER_RENDER_ENGINE   "CS"
#define MOS_COMMAND_BUFFER_VIDEO_ENGINE    "VCS"
#define MOS_COMMAND_BUFFER_VEBOX_ENGINE    "VECS"
#define MOS_COMMAND_BUFFER_TEE_ENGINE      "TEE"
#define MOS_COMMAND_BUFFER_PLATFORM_LEN    4
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#ifndef E_NOTIMPL
#define E_NOTIMPL       0x80004001L
#endif // E_NOTIMPL

#ifndef E_UNEXPECTED
#define E_UNEXPECTED    0x8000FFFFL
#endif // E_UNEXPECTED

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

typedef enum _MOS_SCALABILITY_ENABLE_MODE
{
    MOS_SCALABILITY_ENABLE_MODE_FALSE      = 0,
    MOS_SCALABILITY_ENABLE_MODE_DEFAULT    = 0x0001,
    MOS_SCALABILITY_ENABLE_MODE_USER_FORCE = 0x0010
} MOS_SCALABILITY_ENABLE_MODE;

typedef enum _TRINITY_PATH
{
    TRINITY_DISABLED  = 0,
    TRINITY9_ENABLED  = 1,
    TRINITY11_ENABLED = 2,
} TRINITY_PATH;

#if (_DEBUG || _RELEASE_INTERNAL)
//!
//! \brief for forcing VDBOX
//!

#define    MOS_FORCE_VDBOX_NONE     0
#define    MOS_FORCE_VDBOX_1        0x0001
#define    MOS_FORCE_VDBOX_2        0x0002
//below is for scalability case,
//format is FE vdbox is specified as lowest 4 bits; BE0 is 2nd low 4 bits; BE1 is 3rd low 4bits.
#define    MOS_FORCE_VDBOX_1_1_2    0x0211
//FE-VDBOX2, BE0-VDBOX1, BE2-VDBOX2
#define    MOS_FORCE_VDBOX_2_1_2    0x0212

//!
//! \brief Enum for forcing VEBOX
//!
typedef enum _MOS_FORCE_VEBOX
{
    MOS_FORCE_VEBOX_NONE    = 0,
    MOS_FORCE_VEBOX_1       = 0x0001,
    MOS_FORCE_VEBOX_2       = 0x0002,
    MOS_FORCE_VEBOX_3       = 0x0003,
    MOS_FORCE_VEBOX_4       = 0x0004,
    // For scalability case
    MOS_FORCE_VEBOX_1_2     = 0x0012,
    MOS_FORCE_VEBOX_1_2_3   = 0x0123,
    MOS_FORCE_VEBOX_1_2_3_4 = 0x1234
} MOS_FORCE_VEBOX;

#define MOS_FORCEVEBOX_VEBOXID_BITSNUM              4 //each VEBOX ID occupies 4 bits see defintion MOS_FORCE_VEBOX
#define MOS_FORCEVEBOX_MASK                         0xf


#define MOS_FORCEVDBOX_VDBOXID_BITSNUM              4 //each VDBOX ID occupies 4 bits see defintion MOS_FORCE_VDBOX
#define MOS_FORCEVDBOX_MASK                         0xF

#define MOS_FORCEENGINE_MASK                         0xf
#define MOS_FORCEENGINE_ENGINEID_BITSNUM             4 //each VDBOX ID occupies 4 bits see defintion MOS_FORCE_VDBOX
#define MOS_INVALID_FORCEENGINE_VALUE                0xffffffff
#define MOS_INVALID_ENGINE_INSTANCE                  0xff // this invalid engine instance value aligns with KMD
#endif

typedef struct _MOS_VIRTUALENGINE_INTERFACE *PMOS_VIRTUALENGINE_INTERFACE;
typedef struct _MOS_CMD_BUF_ATTRI_VE MOS_CMD_BUF_ATTRI_VE, *PMOS_CMD_BUF_ATTRI_VE;


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
    PMOS_RESOURCE               resMediaFrameTrackingSurface;
    int32_t                     bUmdSSEUEnable;
    int32_t                     bFrequencyBoost;
    void*                       pAttriVe;
} MOS_COMMAND_BUFFER_ATTRIBUTES, *PMOS_COMMAND_BUFFER_ATTRIBUTES;

//!
//! \brief VDBOX indices
//!
typedef enum _MOS_VDBOX_NODE_IND
{
    MOS_VDBOX_NODE_INVALID     = -1,
    MOS_VDBOX_NODE_1           = 0x0,
    MOS_VDBOX_NODE_2           = 0x1
} MOS_VDBOX_NODE_IND;

//!
//! \brief VEBOX indices
//!
typedef enum _MOS_VEBOX_NODE_IND
{
    MOS_VEBOX_NODE_INVALID     = -1,
    MOS_VEBOX_NODE_1           = 0x0,
    MOS_VEBOX_NODE_2           = 0x1,
    MOS_VEBOX_NODE_3           = 0x2,
    MOS_VEBOX_NODE_4           = 0x3
} MOS_VEBOX_NODE_IND;

#define SUBMISSION_TYPE_SINGLE_PIPE                     (1 << 0)
#define SUBMISSION_TYPE_SINGLE_PIPE_MASK                (0xff)
#define SUBMISSION_TYPE_MULTI_PIPE_SHIFT                8
#define SUBMISSION_TYPE_MULTI_PIPE_ALONE                (1 << SUBMISSION_TYPE_MULTI_PIPE_SHIFT)
#define SUBMISSION_TYPE_MULTI_PIPE_MASTER               (1 << (SUBMISSION_TYPE_MULTI_PIPE_SHIFT+1))
#define SUBMISSION_TYPE_MULTI_PIPE_SLAVE                (1 << (SUBMISSION_TYPE_MULTI_PIPE_SHIFT+2))
#define SUBMISSION_TYPE_MULTI_PIPE_MASK                 (0xff << SUBMISSION_TYPE_MULTI_PIPE_SHIFT)
#define SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT    16
#define SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_MASK     (0xff << SUBMISSION_TYPE_MULTI_PIPE_SLAVE_INDEX_SHIFT)
#define SUBMISSION_TYPE_MULTI_PIPE_FLAGS_SHIFT          24
#define SUBMISSION_TYPE_MULTI_PIPE_FLAGS_MASK           (0xff << SUBMISSION_TYPE_MULTI_PIPE_FLAGS_SHIFT)
#define SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE      (1 << SUBMISSION_TYPE_MULTI_PIPE_FLAGS_SHIFT)
typedef int32_t MOS_SUBMISSION_TYPE;

#define EXTRA_PADDING_NEEDED                            4096
#define MEDIA_CMF_UNCOMPRESSED_WRITE                    0xC

//!
//! \brief Structure to command buffer
//!
typedef struct _MOS_COMMAND_BUFFER
{
    MOS_RESOURCE        OsResource;                 //!< OS Resource

    // Common fields
    uint32_t            *pCmdBase;                   //!< Base    address (CPU)
    uint32_t            *pCmdPtr;                    //!< Current address (CPU)
    int32_t             iOffset;                     //!< Current offset in bytes
    int32_t             iRemaining;                  //!< Remaining size
    int32_t             iTokenOffsetInCmdBuf;        //!< Pointer to (Un)Secure token's next field Offset
    int32_t             iCmdIndex;                   //!< command buffer's index
    MOS_VDBOX_NODE_IND  iVdboxNodeIndex;             //!< Which VDBOX buffer is binded to
    MOS_VEBOX_NODE_IND  iVeboxNodeIndex;             //!< Which VEBOX buffer is binded to
    int32_t             iSubmissionType;
    bool                is1stLvlBB;                  //!< indicate it's a first level BB or not
    struct _MOS_COMMAND_BUFFER   *cmdBuf1stLvl;      //!< Pointer to 1st level command buffer.
    MOS_COMMAND_BUFFER_ATTRIBUTES Attributes;        //!< Attributes for the command buffer to be provided to KMD at submission
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
            uint32_t DumpBeforeSubmit    : 1;                                    //!< Lock only for dump before submit
            uint32_t DumpAfterSubmit     : 1;                                    //!< Lock only for dump after submit
            uint32_t Reserved            : 23;                                   //!< Reserved for expansion.
        };
        uint32_t    Value;
    };
} MOS_LOCK_PARAMS, *PMOS_LOCK_PARAMS;

//!
//! \brief flags for GFX allocation destroy
//!
typedef struct _MOS_GFXRES_FREE_FLAGS
{
    union
    {
        struct
        {
            uint32_t AssumeNotInUse : 1;
            uint32_t SynchronousDestroy : 1;
            uint32_t Reserved : 30;
        };
        uint32_t Value;
    };
} MOS_GFXRES_FREE_FLAGS;

//!
//! \brief Structure to Resource Flags
//!
typedef struct _MOS_GFXRES_FLAGS
{
    int32_t         bNotLockable;                                               //!< [in] true: Resource will not be CPU accessible. false: Resource can be CPU accessed.
    int32_t         bOverlay;
    int32_t         bFlipChain;
    int32_t         bSVM;
    int32_t         bCacheable;
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
    MOS_TILE_MODE_GMM   m_tileModeByForce;                                      //!< [in] Indicates a tile Encoding (aligned w/ GMM defination) needs set by force
    MOS_FORMAT          Format;                                                 //!< [in] Pixel format
    void                *pSystemMemory;                                         //!< [in] Optional parameter. If non null, TileType must be set to linear.
    const char          *pBufName;                                              //!< [in] Optional parameter. A string indicates the buffer name and is used for debugging. nullptr is OK.
    int32_t             bIsCompressible;                                        //!< [in] Resource is compressible or not.
    MOS_RESOURCE_MMC_MODE   CompressionMode;                                    //!< [in] Compression mode.
    int32_t             bIsPersistent;                                          //!< [in] Optional parameter. Used to indicate that resource can not be evicted
    int32_t             bBypassMODImpl;
    int32_t             dwMemType;                                              //!< [in] Optional paramerter. Prefer memory type
    MOS_HW_RESOURCE_DEF ResUsageType;                                           //!< [in] the resource usage type to determine the cache policy
    bool                hardwareProtected;                                      //!< [in] Flag as hint that Resource can be used as hw protected
} MOS_ALLOC_GFXRES_PARAMS, *PMOS_ALLOC_GFXRES_PARAMS;

//!
//! \brief Enum for MOS patch type
//!
typedef enum _MOS_PATCH_TYPE
{
    MOS_PATCH_TYPE_BASE_ADDRESS = 0,
    MOS_PATCH_TYPE_PITCH,
    MOS_PATCH_TYPE_UV_Y_OFFSET,
    MOS_PATCH_TYPE_BIND_ONLY,
    MOS_PATCH_TYPE_UV_BASE_ADDRESS,
    MOS_PATCH_TYPE_V_BASE_ADDRESS,
    MOS_PATCH_TYPE_V_Y_OFFSET
} MOS_PATCH_TYPE;

//!
//! \brief Structure to OS sync parameters
//!
typedef struct _MOS_PATCH_ENTRY_PARAMS
{
    PMOS_RESOURCE presResource;      //!< resource to be patched
    uint32_t      uiAllocationIndex;
    uint32_t      uiResourceOffset;  //!< resource offset
    uint32_t      uiPatchOffset;     //!< patch offset
    uint32_t      bWrite;            //!< is write operation
    int32_t       bUpperBoundPatch;  //!< is upper bound patch
    MOS_HW_COMMAND              HwCommandType;     //!< hw cmd type
    uint32_t                    forceDwordOffset;  //!< force dword offset
    uint8_t                     *cmdBufBase; //!< cmd buffer base address
    uint32_t                    offsetInSSH; //!< patch offset in SSH
    uint32_t                    shiftAmount; //!< shift amount for patch
    uint32_t                    shiftDirection; //!< shift direction for patch
    MOS_PATCH_TYPE              patchType;   //!< patch type
    MOS_COMMAND_BUFFER          *cmdBuffer;  //!< command buffer
} MOS_PATCH_ENTRY_PARAMS, *PMOS_PATCH_ENTRY_PARAMS;

typedef struct _MOS_GPUCTX_CREATOPTIONS MOS_GPUCTX_CREATOPTIONS, *PMOS_GPUCTX_CREATOPTIONS;
struct _MOS_GPUCTX_CREATOPTIONS
{
    uint32_t CmdBufferNumScale;
    uint32_t RAMode;
    uint32_t ProtectMode;
    uint32_t gpuNode;
    //For slice shutdown
    union
    {
        struct
        {
            uint8_t SliceCount;
            uint8_t SubSliceCount;  //Subslice count per slice
            uint8_t MaxEUcountPerSubSlice;
            uint8_t MinEUcountPerSubSlice;
        } packed;

        uint32_t SSEUValue;
    };

    uint8_t isRealTimePriority;  // 1 if context is created from real time priority command queue (run GT at higher frequency)

    _MOS_GPUCTX_CREATOPTIONS() : CmdBufferNumScale(MOS_GPU_CONTEXT_CREATE_DEFAULT),
        RAMode(0),
        ProtectMode(0),
        gpuNode(0),
        SSEUValue(0),
        isRealTimePriority(0){}

    _MOS_GPUCTX_CREATOPTIONS(_MOS_GPUCTX_CREATOPTIONS* createOption) : CmdBufferNumScale(createOption->CmdBufferNumScale),
                                 RAMode(createOption->RAMode),
                                 ProtectMode(createOption->ProtectMode),
                                 gpuNode(createOption->gpuNode),
                                 SSEUValue(createOption->SSEUValue),
                                 isRealTimePriority(createOption->isRealTimePriority) {}

    virtual ~_MOS_GPUCTX_CREATOPTIONS(){}
};

class OsContext;

#if MOS_COMMAND_RESINFO_DUMP_SUPPORTED
//!
//! \brief Class to Dump GPU Command Info
//!
class GpuCmdResInfoDump
{
public:

    static const GpuCmdResInfoDump *GetInstance(PMOS_CONTEXT mosCtx);
    GpuCmdResInfoDump(PMOS_CONTEXT mosCtx);

    void Dump(PMOS_INTERFACE pOsInterface) const;

    void StoreCmdResPtr(PMOS_INTERFACE pOsInterface, const void *pRes) const;

    void ClearCmdResPtrs(PMOS_INTERFACE pOsInterface) const;

private:

    struct GpuCmdResInfo;

    void Dump(const void *pRes, std::ofstream &outputFile) const;

    const std::vector<const void *> &GetCmdResPtrs(PMOS_INTERFACE pOsInterface) const;

    const char *GetResType(MOS_GFXRES_TYPE resType) const;

    const char *GetTileType(MOS_TILE_TYPE tileType) const;

private:

    static std::shared_ptr<GpuCmdResInfoDump> m_instance;
    mutable uint32_t         m_cnt         = 0;
    bool                     m_dumpEnabled = false;
    std::string              m_path        = "";
};
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

class OsContextNext;
typedef void *      OS_PER_STREAM_PARAMETERS;
typedef void *      EXTRA_PARAMS;
typedef OsContextNext OsDeviceContext;
typedef _MOS_GPUCTX_CREATOPTIONS GpuContextCreateOption;
struct _MOS_INTERFACE;
class MosVeInterface;
class CommandList;
class CmdBufMgrNext;
class MosCpInterface;
class MosDecompression;

//!
//! \brief Structure to Unified  InDirectState Dump Info
//!
struct INDIRECT_STATE_INFO
{
    uint32_t        stateSize              = 0;        //size of indirectstate
    uint32_t        *indirectState         = nullptr;  //indirectstate address
    uint32_t        *gfxAddressBottom      = nullptr;  //indirect gfx address bottom
    uint32_t        *gfxAddressTop         = nullptr;  //indirect gfx address top
    const char      *stateName             = "";
};

struct MosStreamState
{
    OsDeviceContext     *osDeviceContext        = nullptr;
    GPU_CONTEXT_HANDLE  currentGpuContextHandle = MOS_GPU_CONTEXT_INVALID_HANDLE;
    MOS_COMPONENT       component               = COMPONENT_UNKNOWN;

    CommandList        *currentCmdList          = nullptr;  //<! Command list used in async mode
    CmdBufMgrNext      *currentCmdBufMgr        = nullptr;  //<! Cmd buffer manager used in async mode
    MosDecompression   *mosDecompression        = nullptr;  //<! Decompression State used for decompress
    bool                enableDecomp            = false;    //<! Set true in StreamState init. If false then not inited
    bool                postponedExecution      = false;    //!< Indicate if the stream is work in postponed execution mode. This flag is only used in aync mode.

    bool supportVirtualEngine                   = false;    //!< Flag to indicate using virtual engine interface
    MosVeInterface *virtualEngineInterface      = nullptr;  //!< Interface to virtual engine state
    bool useHwSemaForResSyncInVe                = false;    //!< Flag to indicate if UMD need to send HW sema cmd under this OS when there is a resource sync need with Virtual Engine interface
    bool veEnable                               = false;    //!< Flag to indicate virtual engine enabled (Can enable VE without using virtual engine interface)
    bool phasedSubmission                       = false;    //!< Flag to indicate if secondary command buffers are submitted together or separately due to different OS
    bool frameSplit                             = true;     //!< Flag to indicate if frame split is enabled (only active when phasedSubmission is true)
    int32_t hcpDecScalabilityMode               = 0;        //!< Hcp scalability mode
    int32_t veboxScalabilityMode                = 0;        //!< Vebox scalability mode

    bool ctxBasedScheduling                     = false;    //!< Indicate if context based scheduling is enabled in this stream
    bool multiNodeScaling                       = false;    //!< Flag to indicate if multi-node scaling is enabled for virtual engine (only active when ctxBasedScheduling is true)

    int32_t ctxPriority                         = 0;
    bool softReset                              = false;    //!< trigger soft reset

    MosCpInterface *osCpInterface               = nullptr;  //!< CP interface

    bool mediaReset                             = false;    //!< Flag to indicate media reset is enabled

    bool forceMediaCompressedWrite              = false;    //!< Flag to force media compressed write

    bool enableDecodeLowLatency                 = false;    //!< Flag to enable decode low latency by frequency boost

    bool simIsActive                            = false;    //!< Flag to indicate if Simulation is enabled
    MOS_NULL_RENDERING_FLAGS nullHwAccelerationEnable = {}; //!< To indicate which components to enable Null HW support

    bool usesPatchList                          = false;    //!< Uses patch list instead of graphic address directly
    bool usesGfxAddress                         = false;    //!< Uses graphic address directly instead of patch list
    bool enableKmdMediaFrameTracking            = false;    //!< Enable KMD Media frame tracking
    bool usesCmdBufHeaderInResize               = false;    //!< Use cmd buffer header in resize
    bool usesCmdBufHeader                       = false;    //!< Use cmd buffer header

    // GPU Reset Statistics
    uint32_t gpuResetCount                      = 0;
    uint32_t gpuActiveBatch                     = 0;
    uint32_t gpuPendingBatch                    = 0;

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    // Command buffer dump
    bool  dumpCommandBuffer                     = false;    //!< Flag to indicate if Dump command buffer is enabled
    bool  dumpCommandBufferToFile               = false;    //!< Indicates that the command buffer should be dumped to a file
    bool  dumpCommandBufferAsMessages           = false;    //!< Indicates that the command buffer should be dumped via MOS normal messages
    char  sDirName[MOS_MAX_HLT_FILENAME_LEN]    = {0};      //!< Dump Directory name - maximum 260 bytes length
    std::vector<INDIRECT_STATE_INFO> indirectStateInfo                     = {};
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if _DEBUG || _RELEASE_INTERNAL
    bool  enableDbgOvrdInVirtualEngine          = false;    //!< enable debug override in virtual engine

    int32_t eForceVdbox                         = 0;        //!< Force select Vdbox
    int32_t eForceVebox                         = 0;        //!< Force select Vebox
#endif // _DEBUG || _RELEASE_INTERNAL

    bool  bParallelSubmission                        = false;    //!< Flag to indicate if parallel submission is enabled
    OS_PER_STREAM_PARAMETERS  perStreamParameters = nullptr; //!< Parameters of OS specific per stream

    static void *pvSoloContext;                             //!< pointer to MediaSolo context

#if MOS_MEDIASOLO_SUPPORTED

    int32_t  bSupportMediaSoloVirtualEngine     = 0;        //!< Flag to indicate if MediaSolo uses VE solution in cmdbuffer submission.
    uint32_t dwEnableMediaSoloFrameNum          = 0;        //!< The frame number at which MediaSolo will be enabled, 0 is not valid.
    int32_t  bSoloInUse                         = 0;        //!< Flag to indicate if MediaSolo is enabled
#endif  // MOS_MEDIASOLO_SUPPORTED

};

// OS agnostic MOS objects
typedef OsDeviceContext *MOS_DEVICE_HANDLE;
typedef MosStreamState  *MOS_STREAM_HANDLE;
//typedef uint32_t             GPU_CONTEXT_HANDLE;
typedef MOS_COMMAND_BUFFER *COMMAND_BUFFER_HANDLE;
typedef MOS_RESOURCE       *MOS_RESOURCE_HANDLE;
typedef MosVeInterface     *MOS_VE_HANDLE;

// OS specific MOS objects
typedef void *              OsSpecificRes;       //!< stand for different os specific resource structure (or info)
typedef void *              OS_HANDLE;           //!< stand for different os handles
typedef MOS_SURFACE         MosResourceInfo;
typedef void *              DDI_DEVICE_CONTEXT;  //!< stand for different os specific device context
typedef void *              MOS_INTERFACE_HANDLE;

class GpuContextMgr;

namespace CMRT_UMD
{
    class CmDevice;
};
struct _CM_HAL_STATE;
typedef struct _CM_HAL_STATE *PCM_HAL_STATE;
class MhwCpInterface;
class CodechalSecureDecodeInterface;
class CodechalSetting;
class CodechalHwInterface;
class CodechalHwInterfaceNext;

struct MOS_SURF_DUMP_SURFACE_DEF
{
    uint32_t offset;  //!< Offset from start of the plane
    uint32_t height;  //!< Height in rows
    uint32_t width;   //!< Width in bytes
    uint32_t pitch;   //!< Pitch in bytes
};

struct ResourceDumpAttri
{
    MOS_RESOURCE            res           = {};
    MOS_LOCK_PARAMS         lockFlags     = {};
    std::string             fullFileName  = {};
    uint32_t                width         = 0;
    uint32_t                height        = 0;
    uint32_t                pitch         = 0;
    MOS_GFXRES_FREE_FLAGS   resFreeFlags  = {};
    MOS_PLANE_OFFSET        yPlaneOffset  = {};  // Y surface plane offset
    MOS_PLANE_OFFSET        uPlaneOffset  = {};  // U surface plane offset
    MOS_PLANE_OFFSET        vPlaneOffset  = {};  // V surface plane offset
};

//!
//! \brief Structure to Unified HAL OS resources
//!
typedef struct _MOS_INTERFACE
{
    //APO WRAPPER
    MOS_STREAM_HANDLE osStreamState = MOS_INVALID_HANDLE; 

    // Saved OS context
    PMOS_CONTEXT                    pOsContext;
    MOS_GPU_CONTEXT                 CurrentGpuContextOrdinal = MOS_GPU_CONTEXT_MAX;
    //!< An internal handle that indexes into the list of GPU Context object
    uint32_t                        CurrentGpuContextHandle;
    //!< A handle to the graphics context device that can be used to calls back
    //!< into the kernel subsystem
    HANDLE                          CurrentGpuContextRuntimeHandle;

    //!< Only used in async and softlet mos mode for backward compatiable
    GPU_CONTEXT_HANDLE              m_GpuContextHandleMap[MOS_GPU_CONTEXT_MAX] = {0};
    GPU_CONTEXT_HANDLE              m_encContext;
    GPU_CONTEXT_HANDLE              m_pakContext;
    // OS dependent settings, flags, limits
    int32_t                         b64bit;
    int32_t                         bDeallocateOnExit;
    MOS_USER_FEATURE_INTERFACE      UserFeatureInterface;
    MosCpInterface                  *osCpInterface;

    int32_t                         bUsesCmdBufHeader;
    int32_t                         bUsesCmdBufHeaderInResize;
    int32_t                         bEnableKmdMediaFrameTracking;
    int32_t                         bNoParsingAssistanceInKmd;
    bool                            bPitchAndUVPatchingNeeded = false;
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
    int32_t                         bAllowExtraPatchToSameLoc;              // patch another resource to same location in cmdbuffer

    // Component info
    MOS_COMPONENT                   Component;

    // Stream info
    uint32_t                        streamIndex = 0;

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
    TRINITY_PATH                    trinityPath;

    bool                            umdMediaResetEnable;

    bool                            forceMediaCompressedWrite;

#if MOS_MEDIASOLO_SUPPORTED
    // MediaSolo related
    int32_t                         bSoloInUse;                                   //!< Flag to indicate if MediaSolo is enabled
    static void                    *pvSoloContext;                                //!< pointer to MediaSolo context
    static uint32_t                 soloRefCnt;
    uint32_t                        dwEnableMediaSoloFrameNum;                    //!< The frame number at which MediaSolo will be enabled, 0 is not valid.
#endif // MOS_MEDIASOLO_SUPPORTED

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    // Command buffer dump
    int32_t                         bDumpCommandBuffer;                                //!< Flag to indicate if Dump command buffer is enabled
    int32_t                         bDumpCommandBufferToFile;                          //!< Indicates that the command buffer should be dumped to a file
    int32_t                         bDumpCommandBufferAsMessages;                      //!< Indicates that the command buffer should be dumped via MOS normal messages
    char                            sPlatformName[MOS_COMMAND_BUFFER_PLATFORM_LEN];    //!< Platform name - maximum 4 bytes length
    char                            sDirName[MOS_MAX_HLT_FILENAME_LEN];                //!< Dump Directory name - maximum 260 bytes length
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if (_RELEASE_INTERNAL||_DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
    CMRTWorkQueueMngr          *m_pWorkQueueMngr;
    CMRT_WORK_QUEUE_INFO       m_WorkQueueInfo[5];  //IGFX_ABSOLUTE_MAX_ENGINES
#endif
#endif

    bool                            bEnableVdboxBalancing;                            //!< Enable per BB VDBox balancing
#if (_DEBUG || _RELEASE_INTERNAL)
    int                             eForceVdbox;                                  //!< Force select Vdbox
    uint32_t                        dwForceTileYfYs;                              // force to allocate Yf (=1) or Ys (=2), remove after full validation
    int32_t                         bTriggerCodecHang;                            // trigger GPU HANG in codec
    int32_t                         bTriggerVPHang;                               //!< trigger GPU HANG in VP
#endif // (_DEBUG || _RELEASE_INTERNAL)

    bool                            apoMosEnabled;                                //!< apo mos or not
    bool                            apoMosForLegacyRuntime = false;
    std::vector<ResourceDumpAttri>  resourceDumpAttriArray;

    MEMORY_OBJECT_CONTROL_STATE (* pfnCachePolicyGetMemoryObject) (
        MOS_HW_RESOURCE_DEF         Usage,
        GMM_CLIENT_CONTEXT          *pGmmClientContext);

    uint8_t (* pfnCachePolicyGetL1Config) (
            MOS_HW_RESOURCE_DEF         Usage,
            GMM_CLIENT_CONTEXT          *pGmmClientContext);

    MOS_STATUS (* pfnCreateGpuContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext,
        MOS_GPU_NODE                GpuNode,
        PMOS_GPUCTX_CREATOPTIONS    createOption);

    GPU_CONTEXT_HANDLE (*pfnCreateGpuComputeContext) (
        PMOS_INTERFACE          osInterface,
        MOS_GPU_CONTEXT         gpuContext,
        MOS_GPUCTX_CREATOPTIONS *createOption);

    MOS_STATUS (* pfnDestroyGpuContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);
    
    MOS_STATUS (* pfnDestroyGpuContextByHandle) (
        PMOS_INTERFACE              pOsInterface,
        GPU_CONTEXT_HANDLE          gpuContextHandle);

    MOS_STATUS (* pfnDestroyGpuComputeContext) (
        PMOS_INTERFACE              osInterface,
        GPU_CONTEXT_HANDLE          gpuContextHandle);

    MOS_STATUS (* pfnSetGpuContext) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_CONTEXT             GpuContext);

    MOS_STATUS (*pfnSetGpuContextFromHandle)(PMOS_INTERFACE osInterface,
                                             MOS_GPU_CONTEXT contextName,
                                             GPU_CONTEXT_HANDLE contextHandle);

    MOS_STATUS (*pfnSetGpuContextHandle) (
        PMOS_INTERFACE     pOsInterface,
        GPU_CONTEXT_HANDLE gpuContextHandle,
        MOS_GPU_CONTEXT    GpuContext);

#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
    MOS_STATUS(*pfnDestroyGuC) (
        PMOS_INTERFACE              pOsInterface);
    MOS_STATUS(*pfnInitGuC) (
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_NODE                 Engine);
    MOS_STATUS(*pfnSubmitWorkQueue) (
        PMOS_INTERFACE pOsInterface,
        MOS_GPU_NODE Engine,
        uint64_t BatchBufferAddress);
#endif
#endif

    MOS_GPU_CONTEXT (* pfnGetGpuContext) (
        PMOS_INTERFACE              pOsInterface);

    void* (*pfnGetGpuContextbyHandle)(
        PMOS_INTERFACE              pOsInterface,
        const GPU_CONTEXT_HANDLE    gpuContextHandle);

    GMM_CLIENT_CONTEXT* (* pfnGetGmmClientContext) (
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

    MOS_STATUS (* pfnWaitAllCmdCompletion) (
        PMOS_INTERFACE              pOsInterface);

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

    MOS_STATUS (*pfnGetMediaEngineInfo)(
        PMOS_INTERFACE     pOsInterface, MEDIA_ENGINE_INFO &info);

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

    uint32_t (* pfnGetInterfaceVersion) (
        PMOS_INTERFACE              pOsInterface);

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

    void (* pfnAddIndirectState) (
        PMOS_INTERFACE      pOsInterface,
        uint32_t            indirectStateSize,
        uint32_t            *pIndirectState,
        uint32_t            *gfxAddressBottom,
        uint32_t            *gfxAddressTop,
        const char          *stateName);

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

    MOS_STATUS (* pfnSetDecompSyncRes) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               syncResource);

    MOS_STATUS(*pfnDoubleBufferCopyResource) (
        PMOS_INTERFACE        pOsInterface,
        PMOS_RESOURCE         pInputOsResource,
        PMOS_RESOURCE         pOutputOsResource,
        bool                  bOutputCompressed);

    MOS_STATUS(*pfnMediaCopyResource2D) (
        PMOS_INTERFACE        pOsInterface,
        PMOS_RESOURCE         pInputOsResource,
        PMOS_RESOURCE         pOutputOsResource,
        uint32_t              copyPitch,
        uint32_t              copyHeight,
        uint32_t              bpp,
        bool                  bOutputCompressed);

    MOS_STATUS (*pfnMonoSurfaceCopy) (
        PMOS_INTERFACE pOsInterface,
        PMOS_RESOURCE  pInputOsResource,
        PMOS_RESOURCE  pOutputOsResource,
        uint32_t       copyPitch,
        uint32_t       copyHeight,
        uint32_t       copyInputOffset,
        uint32_t       copyOutputOffset,
        bool           bOutputCompressed);

    MOS_STATUS (*pfnVerifyMosSurface) (
        PMOS_SURFACE mosSurface,
        bool        &bIsValid);

    MOS_STATUS(*pfnGetMosContext) (
        PMOS_INTERFACE        pOsInterface,
        PMOS_CONTEXT*         mosContext);

    MOS_STATUS (* pfnFillResource) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pResource,
        uint32_t                    dwSize,
        uint8_t                     iValue);

    MOS_STATUS (*pfnUpdateResourceUsageType) (
        PMOS_RESOURCE           pOsResource,
        MOS_HW_RESOURCE_DEF     resUsageType);

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

    uint64_t (*pfnGetResourceClearAddress)(
        PMOS_INTERFACE pOsInterface,
        PMOS_RESOURCE  pResource);

    MOS_STATUS (* pfnSetPatchEntry) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_PATCH_ENTRY_PARAMS     pParams);

#if MOS_MEDIASOLO_SUPPORTED
    MOS_STATUS (* pfnInitializeMediaSolo) (
        PMOS_INTERFACE              pOsInterface);
#endif // MOS_MEDIASOLO_SUPPORTED

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
        PMOS_RESOURCE               &pOsResource);

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

    MOS_STATUS (* pfnGetMemoryCompressionFormat) (
        PMOS_INTERFACE              pOsInterface,
        PMOS_RESOURCE               pOsResource,
        uint32_t                    *pResMmcFormat);

    MOS_STATUS (* pfnCreateVideoNodeAssociation)(
        PMOS_INTERFACE              pOsInterface,
        int32_t                     bSetVideoNode,
        MOS_GPU_NODE                *pVideoNodeOrdinal);

    MOS_VDBOX_NODE_IND (* pfnGetVdboxNodeId)(
        PMOS_INTERFACE              pOsInterface,
        PMOS_COMMAND_BUFFER         pCmdBuffer);

    MOS_STATUS (* pfnDestroyVideoNodeAssociation)(
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_NODE                VideoNodeOrdinal);

    MOS_STATUS(*pfnSkipResourceSync)(
        PMOS_RESOURCE               pOsResource);

    MOS_STATUS(*pfnSetObjectCapture)(
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

    bool (*pfnIsMultipleCodecDevicesInUse)(
        PMOS_INTERFACE              pOsInterface);

    MOS_STATUS (*pfnSetMultiEngineEnabled)(
        PMOS_INTERFACE pOsInterface,
        MOS_COMPONENT  component,
        bool           enabled);

    MOS_STATUS (*pfnGetMultiEngineStatus)(
        PMOS_INTERFACE pOsInterface,
        PLATFORM      *platform,
        MOS_COMPONENT  component,
        bool          &isMultiDevices,
        bool          &isMultiEngine);

    MOS_GPU_NODE(*pfnGetLatestVirtualNode)(
        PMOS_INTERFACE              pOsInterface,
        MOS_COMPONENT               component);

    void (*pfnSetLatestVirtualNode)(
        PMOS_INTERFACE              pOsInterface,
        MOS_GPU_NODE                node);

    MOS_GPU_NODE (*pfnGetDecoderVirtualNodePerStream)(
        PMOS_INTERFACE pOsInterface);

    void (*pfnSetDecoderVirtualNodePerStream)(
        PMOS_INTERFACE pOsInterface,
        MOS_GPU_NODE   node);

    HANDLE (*pfnGetUmdDmaCompleteEventHandle)(
        PMOS_INTERFACE  osInterface,
        MOS_GPU_CONTEXT gpuContext);

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
    //! \brief    Get Gpu Priority
    //! \param    [in] pOsInterface
    //!           pointer to the current gpu context.
    //! \param    [out] pPriority
    //!           pointer to the priority get from gpu context.
    //!
    void (*pfnGetGpuPriority)(
        PMOS_INTERFACE              pOsInterface,
        int32_t                     *pPriority);

    //!
    //! \brief    Set Gpu Priority
    //!
    //! \param    [in] pOsInterface
    //!           pointer to the current gpu context
    //! \param    [in] priority
    //!           pointer to the priority set to gpu context
    //!
    void (*pfnSetGpuPriority)(
        PMOS_INTERFACE              pOsInterface,
        int32_t                     priority);

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

    //!
    //! \brief    Is Device Async or not
    //! \details  Is Device Async or not.
    //! \param    [in] streamState
    //!           Handle of Os Stream State.
    //! \return   bool
    //!           Return true if is async, otherwise false
    //!
    bool (*pfnIsAsyncDevice)(
        MOS_STREAM_HANDLE           streamState);

    //!
    //! \brief    Mos format to os format
    //! \details  Mos format to os format
    //! \param    [in] format
    //!           Mos format.
    //! \return   uint32_t
    //!           Return OS FORMAT
    //!
    uint32_t (*pfnMosFmtToOsFmt)(
        MOS_FORMAT                  format);

    //!
    //! \brief    Os format to Mos format
    //! \details  Os format to Mos format
    //! \param    [in] format
    //!           Os format.
    //! \return   MOS_FORMAT
    //!           Return MOS FORMAT
    //!
    MOS_FORMAT (*pfnOsFmtToMosFmt)(
        uint32_t                    format);

    //!
    //! \brief    Mos format to GMM format
    //! \details  Mos format to GMM format
    //! \param    [in] format
    //!           Mos format.
    //! \return   GMM_RESOURCE_FORMAT
    //!           Return GMM FORMAT
    //!
    GMM_RESOURCE_FORMAT (*pfnMosFmtToGmmFmt)(
        MOS_FORMAT                  format);

    //!
    //! \brief    GMM format to mos format
    //! \details  GMM format to mos format
    //! \param    [in] format
    //!           GMM_RESOURCE_FORMAT.
    //! \return   MOS_FORMAT
    //!           Return MOS FORMAT
    //!
    MOS_FORMAT (*pfnGmmFmtToMosFmt)(
        GMM_RESOURCE_FORMAT         format);

    //!
    //! \brief    Wait For cmd Completion
    //! \details  [GPU Context Interface] Waiting for the completion of cmd in provided GPU context
    //! \details  Caller: HAL only
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in] gpuCtx
    //!           GpuContext handle of the gpu context to wait cmd completion
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnWaitForCmdCompletion)(
        MOS_STREAM_HANDLE           streamState,
        GPU_CONTEXT_HANDLE          gpuCtx);

    //!
    //! \brief    Get Resource array index
    //! \details  Returns the array index
    //! \param    PMOS_RESOURCE
    //!           [in] Pointer to  MOS_RESOURCE
    //! \return   uint32_t - array index
    //!
    uint32_t (*pfnGetResourceArrayIndex)(
        PMOS_RESOURCE               resource);

    //!
    //! \brief    Setup VE Attribute Buffer
    //! \details  [Cmd Buffer Interface] Setup VE Attribute Buffer into cmd buffer.
    //! \details  Caller: MHW only
    //! \details  This interface is called to setup into cmd buffer.
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [out] cmdBuffer
    //!           Cmd buffer to setup VE attribute.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnSetupAttributeVeBuffer)(
        MOS_STREAM_HANDLE           streamState,
        COMMAND_BUFFER_HANDLE       cmdBuffer);

    //!
    //! \brief    Get VE Attribute Buffer
    //! \details  [Cmd Buffer Interface] Get VE Attribute Buffer from cmd buffer.
    //! \details  Caller: HAL only
    //! \details  This interface is called to get VE attribute buffer from cmd buffer if it contains one.
    //!           If there is no VE attribute buffer returned, it means the cmd buffer has no such buffer
    //!           in current MOS module. It is not error state if it is nullptr.
    //! \param    [out] cmdBuffer
    //!           Cmd buffer to setup VE attribute.
    //! \return   MOS_CMD_BUF_ATTRI_VE*
    //!           Return pointer of VE attribute buffer, nullptr if current cmdBuffer didn't contain attribute.
    //!
    MOS_CMD_BUF_ATTRI_VE* (*pfnGetAttributeVeBuffer)(
        COMMAND_BUFFER_HANDLE       cmdBuffer);

    //!
    //! \brief    Setup commandlist and command pool from os interface
    //! \details  Set the commandlist and commandPool used in this stream from os interface.
    //! \param    [in] osInterface
    //!           pointer to the mos interface
    //! \param    [out] streamState
    //!           Handle of Os Stream State.
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnSetupCurrentCmdListAndPool)(
        PMOS_INTERFACE              osInterface,
        MOS_STREAM_HANDLE           streamState);

    //!
    //! \brief    Get MOS_HW_RESOURCE_DEF
    //! \details  [Resource Interface] Get Mos HW Resource DEF
    //!           Caller: HAL & MHW
    //! \param    [in] gmmResUsage
    //!           Gmm Resource usage as index
    //! \return   MOS_HW_RESOURCE_DEF
    //!           Mos HW resource definition
    //!
    MOS_HW_RESOURCE_DEF (*pfnGmmToMosResourceUsageType)(
        GMM_RESOURCE_USAGE_TYPE     gmmResUsage);

    //!
    //! \brief    Get Adapter Info
    //! \details  [System info Interface] Get Adapter Info
    //! \details  Caller: DDI & HAL
    //! \details  This func is called to differentiate the behavior according to Adapter Info.
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \return   ADAPTER_INFO*
    //!           Read-only Adapter Info got, nullptr if failed to get
    //!
    ADAPTER_INFO* (*pfnGetAdapterInfo)(
        MOS_STREAM_HANDLE           streamState);

    //!
    //! \brief    Surface compression is supported
    //! \details  Surface compression is supported
    //! \param    [in] skuTable
    //!           Pointer to MEDIA_FEATURE_TABLE
    //! \return   bool
    //!           If true, compression is enabled
    //!
    bool (*pfnIsCompressibelSurfaceSupported)(
        MEDIA_FEATURE_TABLE         *skuTable);

    //!
    //! \brief    Destroy Virtual Engine State
    //! \details  [Virtual Engine Interface] Destroy Virtual Engine State of provided streamState
    //! \details  Caller: Hal (Scalability) only
    //! \details  This func is called when a stream (Hal instance) need to destroy a VE state
    //! \details  into provided stream.
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnDestroyVirtualEngineState)(
        MOS_STREAM_HANDLE           streamState);

    //!
    //! \brief    Get resource handle
    //! \details  Get resource handle
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in] osResource
    //!           Pointer to mos resource
    //! \return   uint64_t
    //!           Return resource handle
    //!
    uint64_t (*pfnGetResourceHandle)(
        MOS_STREAM_HANDLE           streamState,
        PMOS_RESOURCE               osResource);

    //!
    //! \brief    Get RT log resource info
    //! \details  Get RT log resource info
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in,out] osResource
    //!           Reference to pointer of mos resource
    //! \param    [in,out] size
    //!           Reference to size
    //! \return   void
    //!
    void (*pfnGetRtLogResourceInfo)(
        PMOS_INTERFACE              osInterface,
        PMOS_RESOURCE               &osResource,
        uint32_t                    &size);

    //!
    //! \brief    OS reset resource
    //! \details  Resets the OS resource
    //! \param    PMOS_RESOURCE resource
    //!           [in] Pointer to OS Resource
    //! \return   void
    //!           Return NONE
    //!
    void (*pfnResetResource)(
        PMOS_RESOURCE               resource);


#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    Get engine count
    //! \details  [Virtual Engine Interface] Get engine count from Virtual Engine State in provided stream
    //! \details  Caller: Hal (Scalability) only
    //! \details  Get engine count from virtual engine state
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \return   uint8_t
    //!           Engine count
    //!
    uint8_t (*pfnGetVeEngineCount)(
        MOS_STREAM_HANDLE           streamState);

    //!
    //! \brief    Get Engine Logic Id
    //! \details  [Virtual Engine Interface] Get engine Logic Id from Virtual Engine State in provided stream
    //! \details  Caller: Hal (Scalability) only
    //! \details  Get engine Logic Id from virtual engine state
    //! \param    [in] streamState
    //!           Handle of Os Stream State
    //! \param    [in] instanceIdx
    //!           Engine instance index
    //! \return   uint8_t
    //!
    uint8_t (*pfnGetEngineLogicIdByIdx)(
        MOS_STREAM_HANDLE           streamState,
        uint32_t                    instanceIdx);

    //!
    //! \brief    Set Gpu Virtual Address for Debug
    //! \details  Manually make page fault
    //!
    //! \param    [in] pResource
    //!           Resource to set Gpu Address
    //! \param    [in] address
    //!           Address to set
    //! \return   MOS_STATUS
    //!
    MOS_STATUS (*pfnSetGpuVirtualAddress)(
        PMOS_RESOURCE               pResource,
        uint64_t                    address);
#endif

#if MOS_MEDIASOLO_SUPPORTED
    //!
    //! \brief    Solo set ready to execute
    //! \details  Solo set ready to execute
    //! \param    [in] osInterface
    //!           Pointer to OsInterface
    //! \param    [in] readyToExecute
    //!           ready to execute
    //! \return   void
    //!
    void (*pfnMosSoloSetReadyToExecute)(
        PMOS_INTERFACE              osInterface,
        bool                        readyToExecute);

    //!
    //! \brief    Solo Check node limitation
    //! \details  Solo Check node limitation
    //! \param    [in] osInterface
    //!           Pointer to OsInterface
    //! \param    [out] pGpuNodeToUse
    //!           Pointer to the GPU node to use
    //! \return   void
    //!
    void (*pfnMosSoloCheckNodeLimitation)(
        PMOS_INTERFACE              osInterface,
        uint32_t                    *pGpuNodeToUse);

    //!
    //! \brief    Set MSDK event handling
    //! \details  Receive MSDK event to be set in mediasolo
    //! \param    [in] osInterface
    //!           Pointer to OsInterface
    //! \param    [out] gpuAppTaskEvent
    //!           Event handle
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnMosSoloSetGpuAppTaskEvent)(
        PMOS_INTERFACE              osInterface,
        HANDLE                      gpuAppTaskEvent);

    //!
    //! \brief    Solo force dump
    //! \details  After dwBufferDumpFrameNum submissions are dumped. Force enable mediasolo debug out dump.
    //!           If no solo interface exists, creates one.
    //! \param    [in] dwBufferDumpFrameNum
    //!           Frame Number of the buffer to dump
    //! \param    [in, out] osInterface
    //!           Pointer to OsInterface
    //! \return   void
    //!
    MOS_STATUS (*pfnMosSoloForceDumps)(
        uint32_t                    dwBufferDumpFrameNum,
        PMOS_INTERFACE              osInterface);

    //!
    //! \brief    Solo Pre Process for Decode
    //! \details  Do solo specific operations before decode.
    //!           Include disable aubload optimization.
    //!           And set dumped resource in Aubcapture.
    //! \param    [in] osInterface
    //!           Indicate if this is first execute call
    //! \param    [in] psDestSurface
    //!           Pointer to the surface to add to Aubcapture dump
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnMosSoloPreProcessDecode)(
        PMOS_INTERFACE              osInterface,
        PMOS_SURFACE                psDestSurface);

    //!
    //! \brief    Solo Post Process for Decode
    //! \details  Do solo specific operations after decode.
    //!           Include remove dumped resource in Aubcapture.
    //! \param    [in] osInterface
    //!           Pointer to OsInterface
    //! \param    [in] psDestSurface
    //!           Pointer to the surface to remove from Aubcapture dump
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnMosSoloPostProcessDecode)(
        PMOS_INTERFACE              osInterface,
        PMOS_SURFACE                psDestSurface);

    //!
    //! \brief    Solo Pre Process for Encode
    //! \details  Do solo specific operations before encode.
    //!           Include set dumped resource in Aubcapture.
    //! \param    [in] osInterface
    //!           Pointer to OsInterface
    //! \param    [in] pBitstreamBuffer
    //!           Pointer to the resource to add to Aubcapture dump
    //! \param    [in] pReconSurface
    //!           Pointer to the surface to add to Aubcapture dump
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnMosSoloPreProcessEncode)(
        PMOS_INTERFACE              osInterface,
        PMOS_RESOURCE               pBitstreamBuffer,
        PMOS_SURFACE                pReconSurface);

    //!
    //! \brief    Solo Post Process for Encode
    //! \details  Do solo specific operations after encode.
    //!           Include remove dumped resource in Aubcapture.
    //! \param    [in] osInterface
    //!           Pointer to OsInterface
    //! \param    [in] pBitstreamBuffer
    //!           Pointer to the resource to remove from Aubcapture dump
    //! \param    [in] pReconSurface
    //!           Pointer to the surface to remove from Aubcapture dump
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnMosSoloPostProcessEncode)(
        PMOS_INTERFACE              osInterface,
        PMOS_RESOURCE               pBitstreamBuffer,
        PMOS_SURFACE                pReconSurface);

    //!
    //! \brief    Solo Disable Aubcapture Optimizations
    //! \details  Solo Disable Aubcapture Optimizations
    //! \param    [in] osInterface
    //!           Pointer to OsInterface pointer list
    //! \param    [in] bFirstExecuteCall
    //!           Indicate if this is first execute call
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS (*pfnMosSoloDisableAubcaptureOptimizations)(
        PMOS_INTERFACE              osInterface,
        int32_t                     bFirstExecuteCall);
#endif
    //!
    //! \brief    Notify shared Stream index
    //!
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \return   void
    //!
    void (*pfnNotifyStreamIndexSharing)(
        PMOS_INTERFACE              pOsInterface);

    //!
    //! \brief    Check if mismatch order programming model supported in current device
    //!
    //! \return   bool
    //!
    bool (*pfnIsMismatchOrderProgrammingSupported)();

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
    MOS_STATUS (*pfnAddCommand)(
        PMOS_COMMAND_BUFFER pCmdBuffer,
        const void          *pCmd,
        uint32_t            dwCmdSize);

    //!
    //! \brief    Check virtual engine is supported
    //! \details  Check virtual engine is supported
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
    //!
    MOS_STATUS (*pfnVirtualEngineSupported)(
        PMOS_INTERFACE osInterface,
        bool           isDecode,
        bool           veDefaultEnable);

    //!
    //! \brief    Retrieve the CachePolicyMemoryObject for a resource
    //! \details  Retrieve the CachePolicyMemoryObject for a resource
    //! \param    PMOS_INTERFACE osInterface
    //!           [in] OS Interface
    //! \param    PMOS_RESOURCE resource
    //!           [in] resource
    //! \return   MEMORY_OBJECT_CONTROL_STATE
    //!           return a value of MEMORY_OBJECT_CONTROL_STATE
    //!
    MEMORY_OBJECT_CONTROL_STATE (*pfnGetResourceCachePolicyMemoryObject)(
        PMOS_INTERFACE      osInterface,
        PMOS_RESOURCE       resource);

    //!
    //! \brief    Get Buffer Type
    //! \details  Returns the type of buffer, 1D, 2D or volume
    //! \param    PMOS_RESOURCE pOsResource
    //!           [in] Pointer to OS Resource
    //! \return   GFX resource Type
    //!
    MOS_GFXRES_TYPE (*pfnGetResType)(
        PMOS_RESOURCE pOsResource);

    //!
    //! \brief    Get TimeStamp frequency base
    //! \details  Get TimeStamp frequency base from OsInterface
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \return   uint32_t
    //!           time stamp frequency base
    //!
    uint32_t (*pfnGetTsFrequency)(
        PMOS_INTERFACE         pOsInterface);

    //!
    //! \brief    Set hint parameters
    //! \details  
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] Pointer to OS interface structure
    //! \param    MOS_VIRTUALENGINE_SET_PARAMS veParams
    //!           [out] VIRTUALENGINE SET PARAMS
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
    //!
    MOS_STATUS (*pfnSetHintParams)(
        PMOS_INTERFACE                pOsInterface,
        PMOS_VIRTUALENGINE_SET_PARAMS veParams);

    //!
    //! \brief    Checks whether the requested resource is releasable
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \param    PMOS_RESOURCE pOsResource
    //!           [in] Pointer to OS Resource
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if requested can be released, otherwise MOS_STATUS_UNKNOWN
    //!
    MOS_STATUS (*pfnIsResourceReleasable)(
        PMOS_INTERFACE         pOsInterface,
        PMOS_RESOURCE          pOsResource);

    //!
    //! \brief    Virtual Engine Init for media Scalability
    //! \details  
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] Pointer to OS interface structure
    //! \param    PMOS_VIRTUALENGINE_HINT_PARAMS veHitParams
    //!           [out] Pointer to Virtual Engine hint parameters
    //! \param    PMOS_VIRTUALENGINE_INTERFACE veInterface
    //!           [out] Pointer to Virtual Engine Interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
    //!
    MOS_STATUS (*pfnVirtualEngineInit)(
        PMOS_INTERFACE                  pOsInterface,
        PMOS_VIRTUALENGINE_HINT_PARAMS* veHitParams,
        MOS_VIRTUALENGINE_INIT_PARAMS&  veInParams);
    //!
    //! \brief    initialize virtual engine interface
    //! \details  initialize virtual engine interface
    //! \param    [in]  PMOS_INTERFACE
    //!                pointer to mos interface
    //! \param    [in]  PMOS_VIRTUALENGINE_INIT_PARAMS pVEInitParms
    //!                pointer to VE init parameters
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS (*pfnVirtualEngineInterfaceInitialize)(
        PMOS_INTERFACE                    pOsInterface,
        PMOS_VIRTUALENGINE_INIT_PARAMS    pVEInitParms);

    //!
    //! \brief    Destroy veInterface
    //! \details  
    //! \param    PMOS_VIRTUALENGINE_INTERFACE *veInterface
    //!           [in] Pointer to PMOS_VIRTUALENGINE_INTERFACE
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
    //!
    MOS_STATUS (*pfnDestroyVeInterface)(
        PMOS_VIRTUALENGINE_INTERFACE *veInterface);

    //!
    //! \brief    Creates a CmDevice from a MOS context.
    //! \details  If an existing CmDevice has already associated to the MOS context,
    //!           the existing CmDevice will be returned. Otherwise, a new CmDevice
    //!           instance will be created and associatied with that MOS context.
    //! \param    mosContext
    //!           [in] pointer to MOS conetext.
    //! \param    device
    //!           [in,out] reference to the pointer to the CmDevice.
    //! \param    devCreateOption
    //!           [in] option to customize CmDevice.
    //! \return   int32_t
    //!           CM_SUCCESS if the CmDevice is successfully created.
    //!           CM_NULL_POINTER if pMosContext is null.
    //!           CM_FAILURE otherwise.
    //!
    int32_t (*pfnCreateCmDevice)(
        MOS_CONTEXT             *mosContext,
        CMRT_UMD::CmDevice      *&device,
        uint32_t                devCreateOption,
        uint8_t                 priority);

    //!
    //! \brief    Destroys the CmDevice. 
    //! \details  This function also destroys surfaces, kernels, programs, samplers,
    //!           threadspaces, tasks and the queues that were created using this
    //!           device instance but haven't explicitly been destroyed by calling
    //!           respective destroy functions. 
    //! \param    device
    //!           [in] reference to the pointer to the CmDevice.
    //! \return   int32_t
    //!           CM_SUCCESS if CmDevice is successfully destroyed.
    //!           CM_FAILURE otherwise.
    //!
    int32_t (*pfnDestroyCmDevice)(
        CMRT_UMD::CmDevice      *&device);

    //!
    //! \brief    Initialize cm hal ddi interfaces
    //! \details  Initialize cm hal ddi interfaces
    //! \param    cmState
    //!           [in,out] the pointer to the cm state.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
    //!
    MOS_STATUS (*pfnInitCmInterface)(
        PCM_HAL_STATE           cmState);

    //!
    //! \brief    Create MhwCpInterface Object
    //!           Must use Delete_MhwCpInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \return   Return CP Wrapper Object if CPLIB not loaded
    //!
    MhwCpInterface* (*pfnCreateMhwCpInterface)(PMOS_INTERFACE osInterface);

    //!
    //! \brief    Delete the MhwCpInterface Object
    //!
    //! \param    [in] *pMhwCpInterface
    //!           MhwCpInterface
    //!
    void (*pfnDeleteMhwCpInterface)(MhwCpInterface *mhwCpInterface);

    //!
    //! \brief    Create CodechalSecureDeocde Object
    //!           Must use Delete_CodechalSecureDecodeInterface to delete created Object to avoid ULT Memory Leak errors
    //!
    //! \return   Return CP Wrapper Object
    //!
    CodechalSecureDecodeInterface* (*pfnCreateSecureDecodeInterface)(
        CodechalSetting *codechalSettings,
        CodechalHwInterface *hwInterfaceInput);

    //!
    //! \brief    Delete the CodecHalSecureDecode Object
    //!
    //! \param    [in] *codechalSecureDecodeInterface
    //!           CodechalSecureDecodeInterface
    //!
    void (*pfnDeleteSecureDecodeInterface)(CodechalSecureDecodeInterface *codechalSecureDecodeInterface);

#if (_DEBUG || _RELEASE_INTERNAL)
    //!
    //! \brief    gpuCtxCreateOption Init for media Scalability
    //! \details  
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] Pointer to OS interface structure
    //! \param    uint8_t id
    //!           [out] EngineLogicId
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
    //!
    MOS_STATUS (*pfnGetEngineLogicId)(
        PMOS_INTERFACE                 pOsInterface,
        uint8_t&                       id);
#endif
    //!
    //! \brief    Is Device Async or not
    //! \details  Is Device Async or not.
    //!
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //!
    //! \return   bool
    //!           Return true if is async, otherwise false
    //!
    bool (*pfnIsAsynDevice)(
        PMOS_INTERFACE              osInterface);

    //!
    //! \brief   Get User Setting instance
    //!
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \return   MediaUserSettingSharedPtr
    //!
    MediaUserSettingSharedPtr (*pfnGetUserSettingInstance)(
        PMOS_INTERFACE              pOsInterface);

    bool (*pfnInsertCacheSetting)(CACHE_COMPONENTS id, std::map<uint64_t, MOS_CACHE_ELEMENT> *cacheTablesPtr);

    bool (*pfnGetCacheSetting)(MOS_COMPONENT id, uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_CACHE_ELEMENT &element, bool isHeapSurf);

    // Virtual Engine related
    int32_t                         bSupportVirtualEngine;                        //!< Enable virtual engine flag
    int32_t                         bUseHwSemaForResSyncInVE;                     //!< Flag to indicate if UMD need to send HW sema cmd under this OS when there is a resource sync need with Virtual Engine interface 
    PMOS_VIRTUALENGINE_INTERFACE    pVEInterf;
    bool                            ctxBasedScheduling;                           //!< Flag to indicate if context based scheduling enabled for virtual engine, that is VE2.0.
    bool                            multiNodeScaling;                             //!< Flag to indicate if multi-node scaling is enabled for virtual engine, that is VE3.0.
    bool                            veDefaultEnable = true;                       //!< Flag to indicate if virtual engine is enabled by default
    bool                            phasedSubmission = false;                     //!< Flag to indicate if secondary command buffers are submitted together (Win) or separately (Linux)
    bool                            frameSplit = true;                            //!< Flag to indicate if frame split is enabled
    bool                            bSetHandleInvalid = false;
    bool                            bParallelSubmission = false;                       //!< Flag to indicate if parallel submission is enabled
    MOS_CMD_BUF_ATTRI_VE            bufAttriVe[MOS_GPU_CONTEXT_MAX];

    MOS_STATUS (*pfnCheckVirtualEngineSupported)(
        PMOS_INTERFACE              pOsInterface);

#if MOS_MEDIASOLO_SUPPORTED
    int32_t                         bSupportMediaSoloVirtualEngine;               //!< Flag to indicate if MediaSolo uses VE solution in cmdbuffer submission.
#endif // MOS_MEDIASOLO_SUPPORTED
    bool                            VEEnable;
    bool                            bCanEnableSecureRt;

    int32_t                         bHcpDecScalabilityMode;                       //!< enable scalability decode {mode: default, user force, false}
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_FORCE_VEBOX                 eForceVebox;                                  //!< Force select Vebox
    int32_t                         bEnableDbgOvrdInVE;                           //!< It is for all scalable engines: used together with KMD VE for UMD to specify an engine directly
    int32_t                         bSoftReset;                                   //!< trigger soft reset
#endif // (_DEBUG || _RELEASE_INTERNAL)
    bool streamStateIniter = false;

    int32_t                         bVeboxScalabilityMode;                        //!< Enable scalability vebox

    bool                            bOptimizeCpuTiming;                           //!< Optimize Cpu Timing

    bool                            bNullHwIsEnabled;                             //!< Null Hw is enabled or not

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
//! \param    MOS_CONTEXT_HANDLE pOsDriverContext
//!           [in] Pointer to Driver context
//! \param    MOS_COMPONENT component
//!           [in] OS component
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_InitOsInterface(
    PMOS_INTERFACE      pOsInterface,
    MOS_CONTEXT_HANDLE  pOsDriverContext,
    MOS_COMPONENT       component);

//! \brief    Destroy the mos interface
//! \details  MOS Interface release
//! \param    PMOS_INTERFACE pOsInterface
//!           [in/out] Pointer to OS Interface
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_DestroyInterface(PMOS_INTERFACE pOsInterface);

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

//! \brief    Unified OS get uf path info
//! \details  return a pointer to the user feature key path info
//! \param    PMOS_CONTEXT mosContext
//!           [in/out] Pointer to mosContext
//! \return   MOS_USER_FEATURE_KEY_PATH_INFO*
//!           Return a poniter to user feature path info
//!
MOS_USER_FEATURE_KEY_PATH_INFO *Mos_GetDeviceUfPathInfo(
    PMOS_CONTEXT mosContext);


#if !EMUL
//!
//! \brief    Get memory object based on resource usage
//! \details  Get memory object based on resource usage
//! \param    MOS_HW_RESOURCE_DEF MosUsage
//!           [in] Current usage for resource
//!           [in] Gmm client context
//! \return   MEMORY_OBJECT_CONTROL_STATE
//!           Populated memory object
//!
MEMORY_OBJECT_CONTROL_STATE Mos_CachePolicyGetMemoryObject(
    MOS_HW_RESOURCE_DEF MosUsage,
    GMM_CLIENT_CONTEXT  *pGmmClientContext);

#endif

#ifdef __cplusplus
}
#endif

typedef struct _MOS_GPUCTX_CREATOPTIONS_ENHANCED MOS_GPUCTX_CREATOPTIONS_ENHANCED, *PMOS_GPUCTX_CREATOPTIONS_ENHANCED;
struct _MOS_GPUCTX_CREATOPTIONS_ENHANCED : public _MOS_GPUCTX_CREATOPTIONS
{
    union
    {
        struct
        {
            uint32_t    UsingSFC : 1;
            uint32_t    Reserved1 : 1; // remove HWRestrictedEngine
#if (_DEBUG || _RELEASE_INTERNAL)
            uint32_t    Reserved : 29;
            uint32_t    DebugOverride : 1; // Debug & validation usage only
#else
            uint32_t    Reserved : 30;
#endif
        };
        uint32_t    Flags;
    };

    uint32_t    LRCACount;

#if (_DEBUG || _RELEASE_INTERNAL)
    // Logical engine instances used by this context; valid only if flag DebugOverride is set.
    uint8_t    EngineInstance[MOS_MAX_ENGINE_INSTANCE_PER_CLASS];
#endif

    _MOS_GPUCTX_CREATOPTIONS_ENHANCED()
        : Flags(0),
        LRCACount(0)
    {
#if (_DEBUG || _RELEASE_INTERNAL)
        for (auto i = 0; i < MOS_MAX_ENGINE_INSTANCE_PER_CLASS; i++)
        {
            EngineInstance[i] = MOS_INVALID_ENGINE_INSTANCE;
        }
#endif
    }

    _MOS_GPUCTX_CREATOPTIONS_ENHANCED(_MOS_GPUCTX_CREATOPTIONS* createOption)
        : _MOS_GPUCTX_CREATOPTIONS(createOption)
    {
        if (typeid(*createOption) == typeid(_MOS_GPUCTX_CREATOPTIONS_ENHANCED))
        {
            Flags     = ((MOS_GPUCTX_CREATOPTIONS_ENHANCED *)createOption)->Flags;
            LRCACount = ((MOS_GPUCTX_CREATOPTIONS_ENHANCED *)createOption)->LRCACount;
#if (_DEBUG || _RELEASE_INTERNAL)
            for (auto i = 0; i < MOS_MAX_ENGINE_INSTANCE_PER_CLASS; i++)
            {
                EngineInstance[i] = ((MOS_GPUCTX_CREATOPTIONS_ENHANCED *)createOption)->EngineInstance[i];
            }
#endif
        }
        else
        {
            Flags     = 0;
            LRCACount = 0;
#if (_DEBUG || _RELEASE_INTERNAL)
            for (auto i = 0; i < MOS_MAX_ENGINE_INSTANCE_PER_CLASS; i++)
            {
                EngineInstance[i] = MOS_INVALID_ENGINE_INSTANCE;
            }
#endif
        }
    }
    
};

#define MOS_VE_SUPPORTED(pOsInterface) \
    (pOsInterface ? pOsInterface->bSupportVirtualEngine : false)

#define MOS_VE_CTXBASEDSCHEDULING_SUPPORTED(pOsInterface) \
    (pOsInterface ? pOsInterface->ctxBasedScheduling : false)

#define MOS_VE_MULTINODESCALING_SUPPORTED(pOsInterface) \
    (pOsInterface ? pOsInterface->multiNodeScaling : false)

#if MOS_MEDIASOLO_SUPPORTED
#define MOS_MEDIASOLO_VE_SUPPORTED(pOsInterface) \
    (pOsInterface ? pOsInterface->bSupportMediaSoloVirtualEngine : false)
#endif

__inline void Mos_SetVirtualEngineSupported(PMOS_INTERFACE pOsInterface, bool bEnabled)
{
    if (pOsInterface && pOsInterface->veDefaultEnable)
    {
        pOsInterface->bSupportVirtualEngine = bEnabled;
    }
}

//!
//! \brief   Check whether the parameter of mos surface is valid for copy
//!
//! \param    [in] mosSurface
//!           Pointer to MosSurface
//!
//! \return   bool
//!           Whether the paramter of mosSurface is valid
//!
MOS_STATUS Mos_VerifyMosSurface(
    PMOS_SURFACE mosSurface,
    bool        &bIsValid);

//!
//! \brief    Check virtual engine is supported
//! \details  Check virtual engine is supported
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] OS Interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if succeeded, otherwise error code
//!
MOS_STATUS Mos_CheckVirtualEngineSupported(
    PMOS_INTERFACE osInterface,
    bool           isDecode,
    bool           veDefaultEnable);

//!
//! \brief    Retrieve the CachePolicyMemoryObject for a resource
//! \details  Retrieve the CachePolicyMemoryObject for a resource
//! \param    PMOS_INTERFACE osInterface
//!           [in] OS Interface
//! \param    PMOS_RESOURCE resource
//!           [in] resource
//! \return   MEMORY_OBJECT_CONTROL_STATE
//!           return a value of MEMORY_OBJECT_CONTROL_STATE
//!
MEMORY_OBJECT_CONTROL_STATE Mos_GetResourceCachePolicyMemoryObject(
    PMOS_INTERFACE      osInterface,
    PMOS_RESOURCE       resource);

//!
//! \brief    Is Device Async or not
//! \details  Is Device Async or not.
//! \param    [in] streamState
//!           Handle of Os Stream State.
//! \return   bool
//!           Return true if is async, otherwise false
//!
bool Mos_IsAsyncDevice(
    MOS_STREAM_HANDLE   streamState);

//!
//! \brief    Mos format to os format
//! \details  Mos format to os format
//! \param    [in] format
//!           Mos format.
//! \return   uint32_t
//!           Return OS FORMAT
//!
uint32_t Mos_MosFmtToOsFmt(
    MOS_FORMAT          format);

//!
//! \brief    Os format to Mos format
//! \details  Os format to Mos format
//! \param    [in] format
//!           Os format.
//! \return   MOS_FORMAT
//!           Return MOS FORMAT
//!
MOS_FORMAT Mos_OsFmtToMosFmt(
    uint32_t            format);

//!
//! \brief    Mos format to GMM format
//! \details  Mos format to GMM format
//! \param    [in] format
//!           Mos format.
//! \return   GMM_RESOURCE_FORMAT
//!           Return GMM FORMAT
//!
GMM_RESOURCE_FORMAT Mos_MosFmtToGmmFmt(
    MOS_FORMAT          format);

//!
//! \brief    GMM format to mos format
//! \details  GMM format to mos format
//! \param    [in] format
//!           GMM_RESOURCE_FORMAT.
//! \return   MOS_FORMAT
//!           Return MOS FORMAT
//!
MOS_FORMAT Mos_GmmFmtToMosFmt(
    GMM_RESOURCE_FORMAT format);

//!
//! \brief    Wait For cmd Completion
//! \details  [GPU Context Interface] Waiting for the completion of cmd in provided GPU context
//! \details  Caller: HAL only
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \param    [in] gpuCtx
//!           GpuContext handle of the gpu context to wait cmd completion
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_WaitForCmdCompletion(
    MOS_STREAM_HANDLE   streamState,
    GPU_CONTEXT_HANDLE  gpuCtx);

//!
//! \brief    Get Resource array index
//! \details  Returns the array index
//! \param    PMOS_RESOURCE
//!           [in] Pointer to  MOS_RESOURCE
//! \return   uint32_t - array index
//!
uint32_t Mos_GetResourceArrayIndex(
    PMOS_RESOURCE       resource);

//!
//! \brief    Setup VE Attribute Buffer
//! \details  [Cmd Buffer Interface] Setup VE Attribute Buffer into cmd buffer.
//! \details  Caller: MHW only
//! \details  This interface is called to setup into cmd buffer.
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \param    [out] cmdBuffer
//!           Cmd buffer to setup VE attribute.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_SetupAttributeVeBuffer(
    MOS_STREAM_HANDLE       streamState,
    COMMAND_BUFFER_HANDLE   cmdBuffer);

//!
//! \brief    Get VE Attribute Buffer
//! \details  [Cmd Buffer Interface] Get VE Attribute Buffer from cmd buffer.
//! \details  Caller: HAL only
//! \details  This interface is called to get VE attribute buffer from cmd buffer if it contains one.
//!           If there is no VE attribute buffer returned, it means the cmd buffer has no such buffer
//!           in current MOS module. It is not error state if it is nullptr.
//! \param    [out] cmdBuffer
//!           Cmd buffer to setup VE attribute.
//! \return   MOS_CMD_BUF_ATTRI_VE*
//!           Return pointer of VE attribute buffer, nullptr if current cmdBuffer didn't contain attribute.
//!
MOS_CMD_BUF_ATTRI_VE *Mos_GetAttributeVeBuffer(
    COMMAND_BUFFER_HANDLE   cmdBuffer);

//!
//! \brief    Setup commandlist and command pool from os interface
//! \details  Set the commandlist and commandPool used in this stream from os interface.
//! \param    [in] osInterface
//!           pointer to the mos interface
//! \param    [out] streamState
//!           Handle of Os Stream State.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_SetupCurrentCmdListAndPool(
    PMOS_INTERFACE          osInterface,
    MOS_STREAM_HANDLE       streamState);

//!
//! \brief    Get MOS_HW_RESOURCE_DEF
//! \details  [Resource Interface] Get Mos HW Resource DEF
//!           Caller: HAL & MHW
//! \param    [in] gmmResUsage
//!           Gmm Resource usage as index
//! \return   MOS_HW_RESOURCE_DEF
//!           Mos HW resource definition
//!
MOS_HW_RESOURCE_DEF Mos_GmmToMosResourceUsageType(
    GMM_RESOURCE_USAGE_TYPE gmmResUsage);

//!
//! \brief    Get Adapter Info
//! \details  [System info Interface] Get Adapter Info
//! \details  Caller: DDI & HAL
//! \details  This func is called to differentiate the behavior according to Adapter Info.
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \return   ADAPTER_INFO*
//!           Read-only Adapter Info got, nullptr if failed to get
//!
ADAPTER_INFO *Mos_GetAdapterInfo(
    MOS_STREAM_HANDLE       streamState);

//!
//! \brief    Surface compression is supported
//! \details  Surface compression is supported
//! \param    [in] skuTable
//!           Pointer to MEDIA_FEATURE_TABLE
//! \return   bool
//!           If true, compression is enabled
//!
bool Mos_IsCompressibelSurfaceSupported(
    MEDIA_FEATURE_TABLE     *skuTable);

//!
//! \brief    Destroy Virtual Engine State
//! \details  [Virtual Engine Interface] Destroy Virtual Engine State of provided streamState
//! \details  Caller: Hal (Scalability) only
//! \details  This func is called when a stream (Hal instance) need to destroy a VE state
//! \details  into provided stream.
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS Mos_DestroyVirtualEngineState(
    MOS_STREAM_HANDLE       streamState);

//!
//! \brief    Get resource handle
//! \details  Get resource handle
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \param    [in] osResource
//!           Pointer to mos resource
//! \return   uint64_t
//!           Return resource handle
//!
uint64_t Mos_GetResourceHandle(
    MOS_STREAM_HANDLE       streamState,
    PMOS_RESOURCE           osResource);

//!
//! \brief    Get RT log resource info
//! \details  Get RT log resource info
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \param    [in,out] osResource
//!           Reference to pointer of mos resource
//! \param    [in,out] size
//!           Reference to size
//! \return   void
//!
void Mos_GetRtLogResourceInfo(
    PMOS_INTERFACE          osInterface,
    PMOS_RESOURCE           &osResource,
    uint32_t                &size);

//!
//! \brief    OS reset resource
//! \details  Resets the OS resource
//! \param    PMOS_RESOURCE resource
//!           [in] Pointer to OS Resource
//! \return   void
//!           Return NONE
//!
void Mos_ResetMosResource(
    PMOS_RESOURCE           resource);


bool Mos_InsertCacheSetting(CACHE_COMPONENTS id, std::map<uint64_t, MOS_CACHE_ELEMENT> *cacheTablesPtr);

bool Mos_GetCacheSetting(MOS_COMPONENT id, uint32_t feature, bool bOut, ENGINE_TYPE engineType, MOS_CACHE_ELEMENT &element, bool isHeapSurf);

#if (_DEBUG || _RELEASE_INTERNAL)
//!
//! \brief    Get engine count
//! \details  [Virtual Engine Interface] Get engine count from Virtual Engine State in provided stream
//! \details  Caller: Hal (Scalability) only
//! \details  Get engine count from virtual engine state
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \return   uint8_t
//!           Engine count
//!
uint8_t Mos_GetVeEngineCount(
    MOS_STREAM_HANDLE       streamState);

//!
//! \brief    Get Engine Logic Id
//! \details  [Virtual Engine Interface] Get engine Logic Id from Virtual Engine State in provided stream
//! \details  Caller: Hal (Scalability) only
//! \details  Get engine Logic Id from virtual engine state
//! \param    [in] streamState
//!           Handle of Os Stream State
//! \param    [in] instanceIdx
//!           Engine instance index
//! \return   uint8_t
//!
uint8_t Mos_GetEngineLogicId(
    MOS_STREAM_HANDLE       streamState,
    uint32_t                instanceIdx);

//!
//! \brief    Set Gpu Virtual Address for Debug
//! \details  Manually make page fault
//!
//! \param    [in] pResource
//!           Resource to set Gpu Address
//! \param    [in] address
//!           Address to set
//! \return   MOS_STATUS
//!
MOS_STATUS MOS_SetGpuVirtualAddress(
    PMOS_RESOURCE pResource, 
    uint64_t      address);

#endif

struct ContextRequirement
{
    bool IsEnc               = false;
    bool IsPak               = false;
    bool IsContextSwitchBack = false;
};

#endif  // __MOS_OS_H__
