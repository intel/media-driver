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
//! \file     mos_os.h
//! \brief    Common interface and structure used in MOS OS
//!

#ifndef __MOS_OS_H__
#define __MOS_OS_H__

#include "mos_defs.h"
#include "media_skuwa_specific.h"
#include "mos_utilities.h"
#include "mos_utilities_next.h"
#include "mos_util_debug_next.h"
#include "mos_os_hw.h"         //!< HW specific details that flow through OS pathes
#include "mos_os_cp_interface_specific.h"         //!< CP specific OS functionality 
#if (_RELEASE_INTERNAL || _DEBUG)
#if defined(CM_DIRECT_GUC_SUPPORT)
#include "work_queue_mngr.h"
#include "KmGucClientInterface.h"
#endif
#endif

extern PerfUtility *g_perfutility;

#define PERF_DECODE "DECODE"
#define PERF_ENCODE "ENCODE"
#define PERF_VP "VP"
#define PERF_CP "CP"
#define PERF_MOS "MOS"

#define PERF_LEVEL_DDI "DDI"
#define PERF_LEVEL_HAL "HAL"

#define DECODE_DDI (1)
#define DECODE_HAL (1 << 1)
#define ENCODE_DDI (1 << 4)
#define ENCODE_HAL (1 << 5)
#define VP_DDI     (1 << 8)
#define VP_HAL     (1 << 9)
#define CP_DDI     (1 << 12)
#define CP_HAL     (1 << 13)
#define MOS_DDI    (1 << 16)
#define MOS_HAL    (1 << 17)

#define PERFUTILITY_IS_ENABLED(sCOMP,sLEVEL)                                                              \
    (((sCOMP == "DECODE" && sLEVEL == "DDI") && (g_perfutility->dwPerfUtilityIsEnabled & DECODE_DDI)) ||  \
     ((sCOMP == "DECODE" && sLEVEL == "HAL") && (g_perfutility->dwPerfUtilityIsEnabled & DECODE_HAL)) ||  \
     ((sCOMP == "ENCODE" && sLEVEL == "DDI") && (g_perfutility->dwPerfUtilityIsEnabled & ENCODE_DDI)) ||  \
     ((sCOMP == "ENCODE" && sLEVEL == "HAL") && (g_perfutility->dwPerfUtilityIsEnabled & ENCODE_HAL)) ||  \
     ((sCOMP == "VP" && sLEVEL == "DDI") && (g_perfutility->dwPerfUtilityIsEnabled & VP_DDI)) ||          \
     ((sCOMP == "VP" && sLEVEL == "HAL") && (g_perfutility->dwPerfUtilityIsEnabled & VP_HAL)) ||          \
     ((sCOMP == "CP" && sLEVEL == "DDI") && (g_perfutility->dwPerfUtilityIsEnabled & CP_DDI)) ||          \
     ((sCOMP == "CP" && sLEVEL == "HAL") && (g_perfutility->dwPerfUtilityIsEnabled & CP_HAL)) ||          \
     ((sCOMP == "MOS" && sLEVEL == "DDI") && (g_perfutility->dwPerfUtilityIsEnabled & MOS_DDI)) ||        \
     ((sCOMP == "MOS" && sLEVEL == "HAL") && (g_perfutility->dwPerfUtilityIsEnabled & MOS_HAL)))

#if _RELEASE_INTERNAL
#define PERF_UTILITY_START(TAG,COMP,LEVEL)                                 \
    do                                                                     \
    {                                                                      \
        if (PERFUTILITY_IS_ENABLED((std::string)COMP,(std::string)LEVEL))  \
        {                                                                  \
            g_perfutility->startTick(TAG);                                 \
        }                                                                  \
    } while(0)

#define PERF_UTILITY_STOP(TAG, COMP, LEVEL)                                \
    do                                                                     \
    {                                                                      \
        if (PERFUTILITY_IS_ENABLED((std::string)COMP,(std::string)LEVEL))  \
        {                                                                  \
            g_perfutility->stopTick(TAG);                                  \
        }                                                                  \
    } while (0)

static int perf_count_start = 0;
static int perf_count_stop = 0;

#define PERF_UTILITY_START_ONCE(TAG, COMP,LEVEL)                           \
    do                                                                     \
    {                                                                      \
        if (perf_count_start == 0                                          \
            && PERFUTILITY_IS_ENABLED((std::string)COMP,(std::string)LEVEL))  \
        {                                                                  \
                g_perfutility->startTick(TAG);                             \
        }                                                                  \
        perf_count_start++;                                                \
    } while(0)

#define PERF_UTILITY_STOP_ONCE(TAG, COMP, LEVEL)                           \
    do                                                                     \
    {                                                                      \
        if (perf_count_stop == 0                                           \
            && PERFUTILITY_IS_ENABLED((std::string)COMP,(std::string)LEVEL))  \
        {                                                                  \
            g_perfutility->stopTick(TAG);                                  \
        }                                                                  \
        perf_count_stop++;                                                 \
    } while (0)

#define PERF_UTILITY_AUTO(TAG,COMP,LEVEL) AutoPerfUtility apu(TAG,COMP,LEVEL)

#define PERF_UTILITY_PRINT                         \
    do                                             \
    {                                              \
        if (g_perfutility->dwPerfUtilityIsEnabled) \
        {                                          \
            g_perfutility->savePerfData();         \
        }                                          \
    } while(0)

#else
#define PERF_UTILITY_START(TAG, COMP,LEVEL) do {} while(0)
#define PERF_UTILITY_STOP(TAG, COMP,LEVEL) do {} while(0)
#define PERF_UTILITY_START_ONCE(TAG, COMP,LEVEL) do {} while(0)
#define PERF_UTILITY_STOP_ONCE(TAG, COMP,LEVEL) do {} while(0)
#define PERF_UTILITY_AUTO(TAG,COMP,LEVEL) do {} while(0)
#define PERF_UTILITY_PRINT do {} while(0)
#endif

class AutoPerfUtility
{
public:
    AutoPerfUtility(std::string tag, std::string comp, std::string level);
    ~AutoPerfUtility();

private:
    bool bEnable = false;
    std::string autotag ="intialized";
};

//!
//! \brief OS specific includes and definitions
//!
#include "mos_os_specific.h"
#include "mos_os_virtualengine_specific.h"

#include "mos_oca_interface.h"

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

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
#define MOS_COMMAND_BUFFER_OUT_FILE        "Command_buffer_output"
#define MOS_COMMAND_BUFFER_OUT_DIR         "Command_buffer_dumps"
#define MOS_COMMAND_BUFFER_RENDER_ENGINE   "CS"
#define MOS_COMMAND_BUFFER_VIDEO_ENGINE    "VCS"
#define MOS_COMMAND_BUFFER_VEBOX_ENGINE    "VECS"
#define MOS_COMMAND_BUFFER_RTE_ENGINE      "RTE"
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
    MOS_RESOURCE                resMediaFrameTrackingSurface;
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
    MOS_VDBOX_NODE_IND  iVdboxNodeIndex;            //!< Which VDBOX buffer is binded to
    MOS_VEBOX_NODE_IND  iVeboxNodeIndex;            //!< Which VEBOX buffer is binded to
    int32_t             iSubmissionType;

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
    int32_t             bIsCompressible;                                        //!< [in] Resource is compressible or not.
    MOS_RESOURCE_MMC_MODE   CompressionMode;                                    //!< [in] Compression mode.
    int32_t             bIsPersistent;                                          //!< [in] Optional parameter. Used to indicate that resource can not be evicted
    int32_t             bBypassMODImpl;
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

    _MOS_GPUCTX_CREATOPTIONS() : CmdBufferNumScale(MOS_GPU_CONTEXT_CREATE_DEFAULT),
        RAMode(0),
        gpuNode(0),
        SSEUValue(0){}

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

    static const GpuCmdResInfoDump *GetInstance();
    GpuCmdResInfoDump();

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
    std::string              m_path;
};
#endif // MOS_COMMAND_RESINFO_DUMP_SUPPORTED

class OsContextNext;
typedef void *     OS_PER_STREAM_PARAMETERS;
typedef OsContextNext OsDeviceContext;
typedef _MOS_GPUCTX_CREATOPTIONS GpuContextCreateOption;
struct _MOS_INTERFACE;
class MosVeInterface;

struct MosStreamState
{
    OsDeviceContext   *osDeviceContext = nullptr;
    GPU_CONTEXT_HANDLE currentGpuContextHandle = MOS_GPU_CONTEXT_INVALID_HANDLE;
    MOS_COMPONENT      component;

    MosVeInterface *virtualEngineInterface = nullptr;
    MosCpInterface *osCpInterface = nullptr;

    bool mediaReset    = false;
    uint32_t GpuResetCount = 0;

    bool simIsActive = false;  //!< Flag to indicate if Simulation is enabled
    MOS_NULL_RENDERING_FLAGS nullHwAccelerationEnable; //!< To indicate which components to enable Null HW support

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
    // Command buffer dump
    bool  dumpCommandBuffer = false;                      //!< Flag to indicate if Dump command buffer is enabled
    bool  dumpCommandBufferToFile = false;                //!< Indicates that the command buffer should be dumped to a file
    bool  dumpCommandBufferAsMessages = false;            //!< Indicates that the command buffer should be dumped via MOS normal messages
#endif // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

#if _DEBUG || _RELEASE_INTERNAL
    bool  enableDbgOvrdInVirtualEngine = false;

    int32_t eForceVdbox = 0;   //!< Force select Vdbox
    int32_t eForceVebox = 0;  //!< Force select Vebox
#endif // _DEBUG || _RELEASE_INTERNAL

    bool  ctxBasedScheduling = false;  //!< Indicate if context based scheduling is enabled in this stream
    OS_PER_STREAM_PARAMETERS  perStreamParameters = nullptr; //!< Parameters of OS specific per stream
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

class GpuContextMgr;
//!
//! \brief Structure to Unified HAL OS resources
//!
typedef struct _MOS_INTERFACE
{
    //APO WRAPPER
    MOS_STREAM_HANDLE osStreamState = MOS_INVALID_HANDLE; 

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
    bool                            bPitchAndUVPatchingNeeded;
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

    bool                            umdMediaResetEnable;

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

    GpuContextMgr* (*pfnGetGpuContextMgr) (
        PMOS_INTERFACE     pOsInterface);

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
        PMOS_RESOURCE         pOutputOsResource,
        bool                  bOutputCompressed);

    MOS_STATUS(*pfnMediaCopyResource2D) (
        PMOS_INTERFACE        pOsInterface,
        PMOS_RESOURCE         pInputOsResource,
        PMOS_RESOURCE         pOutputOsResource,
        uint32_t              copyWidth,
        uint32_t              copyHeight,
        uint32_t              copyInputOffset,
        uint32_t              copyOutputOffset,
        bool                  bOutputCompressed);

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

    //!
    //! \brief    Notify shared Stream index
    //!
    //! \param    PMOS_INTERFACE pOsInterface
    //!           [in] OS Interface
    //! \return   void
    //!
    void (*pfnNotifyStreamIndexSharing)(
        PMOS_INTERFACE              pOsInterface);

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
    MOS_CMD_BUF_ATTRI_VE            bufAttriVe[MOS_GPU_CONTEXT_MAX];

    MOS_STATUS (*pfnCheckVirtualEngineSupported)(
        PMOS_INTERFACE              pOsInterface);

#if MOS_MEDIASOLO_SUPPORTED
    int32_t                         bSupportMediaSoloVirtualEngine;               //!< Flag to indicate if MediaSolo uses VE solution in cmdbuffer submission.
#endif // MOS_MEDIASOLO_SUPPORTED
    bool                            VEEnable;
#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_FORCE_VEBOX                 eForceVebox;                                  //!< Force select Vebox
    int32_t                         bHcpDecScalabilityMode;                       //!< enable scalability decode
    int32_t                         bEnableDbgOvrdInVE;                           //!< It is for all scalable engines: used together with KMD VE for UMD to specify an engine directly
    int32_t                         bVeboxScalabilityMode;                        //!< Enable scalability vebox
    int32_t                         bSoftReset;                                   //!< trigger soft reset
#endif // (_DEBUG || _RELEASE_INTERNAL)
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
            EngineInstance[i] = 0xff;
        }
#endif
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

struct ContextRequirement
{
    bool IsEnc = false;
    bool IsPak = false;
};

#endif  // __MOS_OS_H__
