/*
* Copyright (c) 2014-2018, Intel Corporation
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
//! \file      mhw_state_heap.h 
//! \brief         This modules implements HW interface layer to be used on all platforms on     all operating systems/DDIs, across MHW components. 
//!
#ifndef __MHW_STATE_HEAP_H__
#define __MHW_STATE_HEAP_H__

#include "mos_os.h"
#include "mhw_utilities.h"
#include "heap_manager.h"

typedef struct _MHW_STATE_HEAP_MEMORY_BLOCK MHW_STATE_HEAP_MEMORY_BLOCK, *PMHW_STATE_HEAP_MEMORY_BLOCK;
typedef struct _MHW_STATE_HEAP_INTERFACE MHW_STATE_HEAP_INTERFACE, *PMHW_STATE_HEAP_INTERFACE;
typedef class XMHW_STATE_HEAP_INTERFACE *PXMHW_STATE_HEAP_INTERFACE;
typedef struct _MHW_STATE_HEAP MHW_STATE_HEAP, *PMHW_STATE_HEAP;
typedef struct MHW_BLOCK_MANAGER *PMHW_BLOCK_MANAGER;

#define MHW_INDIRECT_HEAP_SIZE      MHW_PAGE_SIZE
#define MHW_SURFACE_PITCH_ALIGNMENT 128
#define MHW_MAX_SURFACE_PLANES      3

#define MHW_NUM_HW_POLYPHASE_TABLES             17
#define MHW_NUM_HW_POLYPHASE_EXTRA_TABLES_G9    15

//!
//! \brief  GSH Defaults and limits
//!
#define MHW_MEDIA_STATE_ALIGN          128
#define MHW_SCRATCH_SPACE_ALIGN        1024
#define MHW_SAMPLER_STATE_ALIGN        64
#define MHW_SAMPLER_STATE_VA_ALIGN     32
#define MHW_SAMPLER_STATE_AVS_ALIGN    1024
#define MHW_SAMPLER_STATE_AVS_ALIGN_G9 2048

#define MHW_SAMPLER_STATE_AVS_ALIGN_MEDIA  1024 // per old HWCMD files AVS samplers were aligned to 256. Not sure if this is needed
#define MHW_SURFACE_STATE_ALIGN 64 // (1 << MHW_BINDING_TABLE_OFFSET_SHIFT)

// Each increment in sampler index represents this increment in offset
#define MHW_SAMPLER_STATE_VA_INC            32
#define MHW_SAMPLER_STATE_AVS_INC_G8        512
#define MHW_SAMPLER_STATE_AVS_INC_G9        2048
#define MHW_SAMPLER_STATE_CONV_INC_G8       512
#define MHW_SAMPLER_STATE_CONV_INC_G9       2048
#define MHW_SAMPLER_STATE_CONV_1D_INC_G9    128

#define MHW_INVALID_BINDING_TABLE_IDX 0xFFFFFFFF

#define MHW_ASSERT_INVALID_BINDING_TABLE_IDX(index)                         \
{                                                                           \
    if ((index) == MHW_INVALID_BINDING_TABLE_IDX)                           \
    {                                                                       \
        MHW_ASSERTMESSAGE("Invalid (nullptr) Pointer.");                    \
        eStatus = MOS_STATUS_UNKNOWN;                                       \
        return eStatus;                                                     \
    }                                                                       \
}

#define MHW_INVALID_SYNC_TAG            0xFFFFFFFF

enum MW_RENDER_ENGINE_ADDRESS_SHIFT
{
    MHW_STATE_HEAP_SURFACE_STATE_SHIFT = 0
};

typedef enum _MHW_STATE_HEAP_PARAM_SHIFTS
{
    MHW_SLM_SHIFT                   = 2,
    MHW_BINDING_TABLE_OFFSET_SHIFT  = 6,
    MHW_BINDING_TABLE_ID_SHIFT      = 5,
    MHW_SAMPLER_SHIFT               = 5,
    MHW_CURBE_SHIFT                 = 5,
    MHW_THRD_CON_DATA_RD_SHIFT      = 5,
    MHW_KERNEL_OFFSET_SHIFT         = 6,
    MHW_SAMPLER_INDIRECT_SHIFT      = 6,
    MHW_SCRATCH_SPACE_SHIFT         = 10,
    MHW_SSH_BASE_SHIFT              = 12,
    MHW_COMPUTE_INDIRECT_SHIFT      = 6
} MHW_STATE_HEAP_PARAM_SHIFTS;

typedef enum _MHW_FRAME_FIELD_TYPE
{
    MHW_FRAME = 0,
    MHW_TOP_FIELD,
    MHW_BOTTOM_FIELD,
    MHW_NUM_FRAME_FIELD_TYPES,
} MHW_FRAME_FIELD_TYPE;

typedef enum _MHW_STATE_HEAP_TYPE
{
    MHW_ISH_TYPE = 0,
    MHW_DSH_TYPE,
    MHW_SSH_TYPE      //!< Note SSH is currently managed by MOS
} MHW_STATE_HEAP_TYPE;

typedef enum _MHW_STATE_HEAP_MODE
{
    MHW_RENDER_HAL_MODE = 0,     //!< 0 - RenderHal handles
    MHW_DSH_MODE,                //!< 1 - MDF dynamic heap management
    MHW_DGSH_MODE                //!< 2 - Ddynamic generic heap management 
}MHW_STATE_HEAP_MODE;

typedef enum _MHW_PLANE
{
    MHW_GENERIC_PLANE = 0,  // 1D Surface: MHW_GENERIC_PLANE, 2D Surface: MHW_Y_PLANE
    MHW_Y_PLANE = 0,
    MHW_U_PLANE,
    MHW_V_PLANE,
} MHW_PLANE;

//!
//! \brief MHW Rotation Mode enum
//!
typedef enum _MHW_ROTATION
{
    MHW_ROTATION_IDENTITY = 0,      //!< Rotation 0 degrees
    MHW_ROTATION_90,                //!< Rotation 90 degrees
    MHW_ROTATION_180,               //!< Rotation 180 degrees
    MHW_ROTATION_270,               //!< Rotation 270 degrees
    MHW_MIRROR_HORIZONTAL,          //!< Horizontal Mirror
    MHW_MIRROR_VERTICAL,            //!< Vertical Mirror
    MHW_ROTATE_90_MIRROR_VERTICAL,  //!< 90 + V Mirror
    MHW_ROTATE_90_MIRROR_HORIZONTAL //!< 90 + H Mirror
} MHW_ROTATION;

//!
//! \brief Render chroma siting vertical value
//!
typedef enum _MHW_CHROMA_SITING_VDIRECTION
{
    MHW_CHROMA_SITING_VDIRECTION_0   = 0x0,
    MHW_CHROMA_SITING_VDIRECTION_1_4 = 0x1,
    MHW_CHROMA_SITING_VDIRECTION_1_2 = 0x2,
    MHW_CHROMA_SITING_VDIRECTION_3_4 = 0x3,
    MHW_CHROMA_SITING_VDIRECTION_1   = 0x4
} MHW_CHROMA_SITING_VDIRECTION;

//!
//! \brief Chroma Siting enum
//!
typedef enum _MHW_CHROMA_SITING
{
    MHW_CHROMA_SITING_NONE          = 0,
    MHW_CHROMA_SITING_HORZ_LEFT     = 1 << 0,
    MHW_CHROMA_SITING_HORZ_CENTER   = 1 << 1,
    MHW_CHROMA_SITING_HORZ_RIGHT    = 1 << 2,
    MHW_CHROMA_SITING_VERT_TOP      = 1 << 4,
    MHW_CHROMA_SITING_VERT_CENTER   = 1 << 5,
    MHW_CHROMA_SITING_VERT_BOTTOM   = 1 << 6,
} MHW_CHROMA_SITING;

//!
//! \brief  AVS Params
//!
typedef struct _MHW_AVS_PARAMS
{
    MOS_FORMAT              Format;
    float                   fScaleX;
    float                   fScaleY;
    int32_t                 *piYCoefsX;
    int32_t                 *piYCoefsY;
    int32_t                 *piUVCoefsX;
    int32_t                 *piUVCoefsY;
    bool                    bForcePolyPhaseCoefs;
} MHW_AVS_PARAMS, *PMHW_AVS_PARAMS;

// Memory block state
typedef enum _MHW_BLOCK_STATE
{
    MHW_BLOCK_STATE_POOL = 0,       //!< Block belongs to the pool of memory block objects, it doesn't point to a valid memory block, most fields may be invalid.
    MHW_BLOCK_STATE_FREE,           //!< Block points to available memory in State Heap
    MHW_BLOCK_STATE_ALLOCATED,      //!< Block points to allocated area in State Heap (allocated but not in use by GPU)
    MHW_BLOCK_STATE_SUBMITTED,      //!< Block points to area in State Heap that was submitted for execution by GPU; memory cannot be overwritten or deleted before workload is finished.
    MHW_BLOCK_STATE_DELETED,        //!< Block is marked for deletion (State Heap is being deleted).

    MHW_BLOCK_STATE_COUNT = 5
} MHW_BLOCK_STATE;

struct _MHW_STATE_HEAP_MEMORY_BLOCK
{
    //!
    //! \brief The sync tag ID for the memory block may be used to determine
    //!        whether or not the memory block is in use--if the ID is
    //!        invalid (+1 to the defined maximum number of sync tags), the
    //!        memory block is not in use and some or all of it may be used
    //!        for a new kernel state region.
    //!
    FrameTrackerTokenFlat trackerToken;

    uint32_t            dwBlockSize;
    PMHW_STATE_HEAP     pStateHeap;
    uint32_t            dwOffsetInStateHeap;
    bool                bStatic;      //!< The kernel state region in this state heap is static and will not be removed during cleanup step.

    _MHW_STATE_HEAP_MEMORY_BLOCK    *pPrev;
    _MHW_STATE_HEAP_MEMORY_BLOCK    *pNext;

    //!
    //! \brief The following code is for the MHW dynamic state heap implementation by MDF. 
    //!        Code is not yet unified across all media driver components
    //!
    MHW_BLOCK_STATE                 BlockState: 16;   //!< Current block state (void, free, allocated, in use, completed, deleted)
    uint32_t                        bDelete   :  1;   //!< Block is flagged for deletion upon completion
    uint32_t                        Reserved  : 15;   //!< Reserved (uniq block ID used for tracking block utilization by system)

    PMHW_STATE_HEAP_MEMORY_BLOCK    pHeapNext;        //!< Next block in same state heap (adjacent), null if last
    PMHW_STATE_HEAP_MEMORY_BLOCK    pHeapPrev;        //!< Previous block in same state heap (adjacent), null if first

    uint8_t                         *pDataPtr;         //!< Pointer to aligned data
    uint32_t                        dwDataOffset;     //!< Offset of pDataPtr (from State Heap Base - used in state programming)
    uint32_t                        dwDataSize;       //!< Data size (>= requested size due to heap granularity)
    uint32_t                        dwAlignment;      //!< Offset alignment (offset from actual block start)
};

typedef struct _MHW_KERNEL_PARAM
{
    void    *pExtra;                         //!< Kernel parameter
    uint8_t *pBinary;                        //!< Pointer to kernel binary
    int32_t iSize;                          //!< Kernel size
    int32_t iGrfCount;                      //!< Number of registers
    int32_t iBTCount;                       //!< Number of BT entries
    int32_t iThreadCount;                   //!< Number of threads (max)
    int32_t iGrfStartRegister;              //!< Start register
    int32_t iSamplerCount;                  //!< Sampler count
    int32_t iSamplerLength;                 //!< Sampler length
    int32_t iCurbeLength;                   //!< Constant URB length
    int32_t iIdCount;                       //!< Num IDs used by kernel state
    int32_t iInlineDataLength;              //!< MEDIA_OBJECT inline data (aka URB length)
    int32_t iBlockWidth;                    //!< Block width
    int32_t iBlockHeight;                   //!< Block height

    //!
    //! \brief Dynamic kernel parameters may follow below if necessary.
    //!
    int32_t bLoaded;                        //!< Kernel Loaded flag
    int32_t iKID;                           //!< Interface descriptor ID for the kernel
    int32_t iKUID;                          //!< Kernel Unique ID
    int32_t iKCID;                          //!< Kernel Cache ID
    int32_t iAKBaseID;                      //!< Authenticated Kernel Base ID

    bool    bForceReload;                   //!< The flag to indicate if the kernel need to be reloaded forcibly

} MHW_KERNEL_PARAM, *PMHW_KERNEL_PARAM;

typedef struct MHW_KERNEL_STATE
{
    MHW_KERNEL_STATE()
    {
        m_dshRegion = MemoryBlock();
        m_ishRegion = MemoryBlock();
    }

    virtual ~MHW_KERNEL_STATE() { MHW_FUNCTION_ENTER;  }
    //!
    //! \brief Set when the kernel state is created
    //!
    MHW_KERNEL_PARAM    KernelParams = {};   //!< Kernel parameters

    //!
    //! \brief Set when the kernel state region for the kernel state
    //!        is acquired.
    //!        Note: For state heaps other than the SSH it is possible to
    //!              make a kernel state region static, such that it belongs
    //!              to the kernel state and it is not necessary to acquire a
    //!              kernel state region for the DSH/ISH (whichever is
    //!              static).
    //!
    uint32_t dwSshOffset = 0;            //!< Offset within SSH to the kernel state region
    uint32_t dwBindingTableSize = 0;     //!< The size of the binding table for this kernel state
    uint32_t dwSshSize = 0;              //!< Size of the kernel state region in the SSH
    uint32_t dwIdOffset = 0;             //!< Offset within DSH to ID(s) in kernel state region
    uint32_t dwCurbeOffset = 0;          //!< Offset within DSH to CURBE(s) in kernel state region
    uint32_t dwSamplerOffset = 0;        //!< Offset within DSH to Sampler(s) in kernel state region
    uint32_t dwKernelBinaryOffset = 0;   //!< Offset within ISH to the kernel state region

    //!
    //! \brief Descriptors of the DSH/ISH kernel state regions. Since there
    //!        may be multiple DSH/ISH buffers, it is necessary to store
    //!        which one is used. If one of the regions is static, the memory
    //!        block pointer is always expected to be valid.
    //!
    MemoryBlock m_dshRegion;
    MemoryBlock m_ishRegion;

    uint32_t    m_currTrackerId = MemoryBlock::m_invalidTrackerId;  //!< tracker ID for the current execution
} *PMHW_KERNEL_STATE;

typedef struct _MHW_BINDING_TABLE_PARAMS {
    uint8_t     *pBindingTableEntry;             // Pointer to BT entry to setup
    uint32_t    dwSurfaceStateOffset;           // Offset to Surface State (Indirect State)
    bool        bSurfaceStateAvs;               // true if AVS surface
    int32_t     iBindingTableEntry;             // Binding Table entry index
} MHW_BINDING_TABLE_PARAMS, *PMHW_BINDING_TABLE_PARAMS;

typedef struct _MHW_BINDING_TABLE_SEND_PARAMS {
    uint8_t     *pBindingTableSource;            // Pointer to BT source
    uint8_t     *pBindingTableTarget;            // Pointer to BT target
    int32_t     iSurfaceStateBase;              // Offset to first Surface State in SSH
    int32_t     iSurfaceStateOffset;            // [out] Offset to Surface State in SSH
    int32_t     iSurfaceState;                  // [out] Surface State index (-1 if Copy==0)
} MHW_BINDING_TABLE_SEND_PARAMS, *PMHW_BINDING_TABLE_SEND_PARAMS;

typedef struct _MHW_SURFACE_STATE_PARAMS {
    uint8_t     *pSurfaceState;
    uint32_t    dwCacheabilityControl;
    uint32_t    dwFormat;
    uint32_t    dwWidth;
    uint32_t    dwHeight;
    uint32_t    dwDepth;
    uint32_t    dwPitch;
    uint32_t    dwQPitch;
    uint32_t    bUseAdvState              : 1;
    uint32_t    AddressControl            : 1;
    uint32_t    SurfaceType3D             : 3;
    uint32_t    bTiledSurface             : 1;
    uint32_t    bTileWalk                 : 1;
    uint32_t    bVerticalLineStride       : 1;
    uint32_t    bVerticalLineStrideOffset : 1;
    uint32_t    bCompressionEnabled       : 1;
    uint32_t    bCompressionMode          : 1;
    uint32_t    MmcState                  : 3;
    uint32_t    bInterleaveChroma         : 1;
    uint32_t    bHalfPitchChroma          : 1;
    uint32_t    bSeperateUVPlane          : 1;
    uint32_t    UVPixelOffsetUDirection   : 2;
    uint32_t    UVPixelOffsetVDirection   : 2;
    uint32_t    RotationMode              : 3;
    uint32_t    bSurfaceArraySpacing      : 1;
    uint32_t    bBoardColorOGL            : 1;
    int32_t     iXOffset;
    int32_t     iYOffset;
    uint32_t    dwXOffsetForU; // U or UV
    uint32_t    dwYOffsetForU; // U or UV
    uint32_t    dwXOffsetForV;
    uint32_t    dwYOffsetForV;
    uint32_t    dwCompressionFormat;    // Memory Compression Format
    uint32_t    L1CacheConfig;

    uint32_t    *pdwCmd;                // [out] Pointer for patching
    uint32_t    dwLocationInCmd;       // [out] Offset in command for patching
    MOS_TILE_MODE_GMM TileModeGMM;     // Tile Type from GMM Definition
    bool        bGMMTileEnabled;       //!<  GMM defined tile mode flag
} MHW_SURFACE_STATE_PARAMS, *PMHW_SURFACE_STATE_PARAMS;

struct _MHW_STATE_HEAP
{
    MOS_RESOURCE    resHeap;        //!< Graphics resource for state heap
    void            *pvLockedHeap;   //!< System (logical) address for state heap
    bool            bKeepLocked;
    uint32_t        dwSize;         //!< Size of the state heap

    uint32_t        dwUsed;         //!< Used memory in state heap
    uint32_t        dwFree;         //!< Free memory in state heap
    bool            bDeleted;       //!< State heap is in process of being deleted

    // State heap object points to its interface object and block manager
    PMHW_BLOCK_MANAGER pBlockManager;
    PXMHW_STATE_HEAP_INTERFACE pMhwStateHeapInterface;

    //!
    //! \brief The memory blocks will be managed in a linked list, each state
    //!        heap will have one linked list starting with pMemoryHead. The
    //!        memory blocks in this list will describe all available and 
    //!        used space in the state heap.
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryHead;

    PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryTail;    //!< Last block in state heap memory (used to traverse pMemNext/pMemPrev)
    PMHW_STATE_HEAP_MEMORY_BLOCK    pDebugKernel;   //!< Block associated to debug (SIP) kernel in the current ISH
    PMHW_STATE_HEAP_MEMORY_BLOCK    pScratchSpace;  //!< Block associated with current active scratch space (older scratch spaces are removed)
    uint32_t                        dwScratchSpace; //!< Active scratch space size

    PMHW_STATE_HEAP  pPrev;         //!< The first state heap is considered primary (pPrev == nullptr)
    PMHW_STATE_HEAP  pNext;

    uint32_t         dwCurrOffset;   //!< For simulated SSH to denote the current amount of space used
};

typedef struct _MHW_SYNC_TAG
{
    uint32_t dwCmdBufId;             //!< Command buffer ID for this sync tag, 0 is available
    uint32_t dwSshSizeUsed;

    //!
    //! \brief Memory blocks used during the clean up step.
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK pDshRegion; //!< Memory block for DSH
    PMHW_STATE_HEAP_MEMORY_BLOCK pIshRegion; //!< Memory block for ISH
} MHW_SYNC_TAG, *PMHW_SYNC_TAG;

typedef struct _MHW_INTERFACE_DESCRIPTOR_PARAMS
{
    PMHW_KERNEL_STATE   pKernelState;
    uint32_t            dwKernelStartOffset; //!< Offset relative to the kernel state region binary start offset in the ISH
    uint32_t            dwIdIdx;             //!< Index within the ID block of the kernel state region in the DSH of the ID to be added
    uint32_t            dwBtOffset;          //!< Offset relative to SSH base of the BT start
    uint32_t            dwSamplerOffset;     //!< Offset within the Sampler block of the kernel state region for the current sampler state
} MHW_INTERFACE_DESCRIPTOR_PARAMS, *PMHW_INTERFACE_DESCRIPTOR_PARAMS;

// Structure used to program media interface descriptor entry
typedef struct _MHW_ID_ENTRY_PARAMS
{
    uint32_t            dwMediaIdOffset;                //! Offset of the first Media Interface Descriptor (in DSH)
    uint32_t            iMediaId;                       //! Media Interface Descriptor #
    uint32_t            dwKernelOffset;                 //! Kernel offset (in ISH)
    uint32_t            dwSamplerOffset;                //! Sampler offset (in DSH)
    uint32_t            dwSamplerCount;                 //! Sample count
    uint32_t            dwBindingTableOffset;           //! Binding table offset (in DSH)
    uint32_t            iCurbeOffset;                   //! Curbe offset (in DSH)
    uint32_t            iCurbeLength;                   //! Curbe lenght
    bool                bBarrierEnable;                 //! Enable Barrier
    bool                bGlobalBarrierEnable;           //! Enable Global Barrier (SKL+)
    uint32_t            dwNumberofThreadsInGPGPUGroup;  //! Number of threads per group
    uint32_t            dwSharedLocalMemorySize;        //! Size of SharedLocalMemory (SLM)
    int32_t             iCrsThdConDataRdLn;             //!
    PMHW_STATE_HEAP     pGeneralStateHeap;              //! General state heap in use
    MemoryBlock         *memoryBlock;                   //! Memory block associated with the state heap
} MHW_ID_ENTRY_PARAMS, *PMHW_ID_ENTRY_PARAMS;

typedef struct _MHW_PLANE_SETTING
{
    uint8_t ui8PlaneID;                                                            // Plane identifier
    uint8_t ui8ScaleWidth;                                                         // X Scale (divider)
    uint8_t ui8ScaleHeight;                                                        // Y Scale (divider)
    uint8_t ui8AlignWidth;                                                         // X Alignment
    uint8_t ui8AlignHeight;                                                        // Y Alignment
    uint8_t ui8PixelsPerDword;                                                     // Pixels per Dword (for dataport read/write)
    bool    bAdvanced;                                                             // Advanced Surface State
    uint32_t dwFormat;                                                              // Hardware Surface Format
} MHW_PLANE_SETTING, *PMHW_PLANE_SETTING;

typedef struct _MHW_SURFACE_PLANES
{
    uint32_t            dwNumPlanes;                                              // Number of planes
    MHW_PLANE_SETTING   Plane[MHW_MAX_SURFACE_PLANES];                            // Description of each plane
} MHW_SURFACE_PLANES, *PMHW_SURFACE_PLANES;

typedef const _MHW_PLANE_SETTING  *PCMHW_PLANE_SETTING;
typedef const _MHW_SURFACE_PLANES *PCMHW_SURFACE_PLANES;

typedef struct _MHW_RCS_SURFACE_PARAMS
{
    PMOS_SURFACE    psSurface;

    uint32_t        bUseAdvState;                     //!< Indicates that SURFACE_STATE_ADV should be used

    uint32_t        dwNumPlanes;                      //!< Indicates the number of valid binding table offsets included
    uint32_t        dwPlaneType[MHW_MAX_SURFACE_PLANES]; //!< Indicates the plane type
    uint32_t        dwBindingTableOffset[MHW_MAX_SURFACE_PLANES]; //!< Binding table offset for all planes included in surface
    uint32_t        dwCacheabilityControl;
    bool            bRenderTarget;
    bool            bIsWritable;

    uint32_t        dwWidthToUse[MHW_MAX_SURFACE_PLANES];   //!< If non-zero, overrides value in psSurface
    uint32_t        dwHeightToUse[MHW_MAX_SURFACE_PLANES];  //!< If non-zero, overrides value in psSurface
    uint32_t        dwPitchToUse[MHW_MAX_SURFACE_PLANES];   //!< If non-zero, overrides value in psSurface
    uint32_t        dwBaseAddrOffset[MHW_MAX_SURFACE_PLANES];
    uint32_t        dwYOffset[MHW_MAX_SURFACE_PLANES];
    uint32_t        dwXOffset[MHW_MAX_SURFACE_PLANES];
    uint32_t        ForceSurfaceFormat[MHW_MAX_SURFACE_PLANES]; //!< Of type GFX3DSTATE_SURFACEFORMAT
    uint32_t        dwSurfaceType; //!< of type GFX3DSTATE_SURFACETYPE

    uint32_t        bVertLineStride;
    uint32_t        bVertLineStrideOffs;
    uint32_t        bInterleaveChroma;

    uint32_t                     dwAddressControl;
    MHW_CHROMA_SITING_VDIRECTION Direction;
    MHW_ROTATION                 Rotation;                         //!<  0: 0 degree, 1: 90 degree, 2: 180 degree, 3: 270 degree
    uint32_t                        MediaBoundaryPixelMode;     //!< Of type GFX3DSTATE_MEDIA_BOUNDARY_PIXEL_MODE
    uint32_t                     dwOffsetInSSH;
} MHW_RCS_SURFACE_PARAMS, *PMHW_RCS_SURFACE_PARAMS;

typedef struct _MHW_SURFACE_TOKEN_PARAMS
{
    PMOS_SURFACE    pOsSurface;
    uint32_t        dwSurfaceOffset;
    uint32_t        YUVPlane            : 2;
    uint32_t        bRenderTarget       : 1;
    uint32_t                            : 1;
    uint32_t        bSurfaceTypeAvs     : 1;
    uint32_t                            : 26;
} MHW_SURFACE_TOKEN_PARAMS, *PMHW_SURFACE_TOKEN_PARAMS;

//!
//! \brief Sampler Type
//!
typedef enum _MHW_SAMPLER_TYPE
{
    MHW_SAMPLER_TYPE_INVALID  = 0,
    MHW_SAMPLER_NONE          ,
    MHW_SAMPLER_TYPE_3D       ,   // UNORM
    MHW_SAMPLER_TYPE_AVS      ,   // AVS (Avanced Video Sampler = 8x8, STE, IEF)
    MHW_SAMPLER_TYPE_VME      ,   // VME
    MHW_SAMPLER_TYPE_MISC     ,   // MISC
    MHW_SAMPLER_TYPE_MINMAX   ,
    MHW_SAMPLER_TYPE_ERODE    ,
    MHW_SAMPLER_TYPE_DILATE   ,
    MHW_SAMPLER_TYPE_CONV         // CONVOLUTION (Gen8+)
} MHW_SAMPLER_TYPE, *PMHW_SAMPLER_TYPE;

typedef enum _MHW_SAMPLER_ELEMENTS_TYPE : unsigned int
{
    MHW_Sampler1Element = 0,
    MHW_Sampler2Elements,
    MHW_Sampler4Elements,
    MHW_Sampler8Elements,
    MHW_Sampler64Elements,
    MHW_Sampler128Elements,

    MHW_SamplerTotalElements
}MHW_SAMPLER_ELEMENT_TYPE, *PMHW_SAMPLER_ELEMENT_TYPE;

//!
//! \brief Sampler Tap Mode
//!
typedef enum _MHW_SAMPLER_TAP_MODE
{
    MHW_SAMPLER_FILTER_4_TAP  = 0,
    MHW_SAMPLER_FILTER_8_4_TAP,
    MHW_SAMPLER_FILTER_8_TAP,
    MHW_SAMPLER_FILTER_8_TAP_ADATIVE
} MHW_SAMPLER_TAP_MODE, *PMHW_SAMPLER_TAP_MODE;

//!
//! \brief Sampler Filter Mode
//!
typedef enum _MHW_SAMPLER_FILTER_MODE
{
    MHW_SAMPLER_FILTER_CUSTOM = 0,
    MHW_SAMPLER_FILTER_NEAREST,
    MHW_SAMPLER_FILTER_BILINEAR
} MHW_SAMPLER_FILTER_MODE, *PMHW_SAMPLER_FILTER_MODE;

typedef enum _MHW_SAMPLER_TEXTADDR
{
    MHW_SAMPLER_TEXTADDR_WRAP,
    MHW_SAMPLER_TEXTADDR_MIRROR,
    MHW_SAMPLER_TEXTADDR_CLAMP,
    MHW_SAMPLER_TEXTADDR_CUBE,
    MHW_SAMPLER_TEXTADDR_CLAMPBORDER,
    MHW_SAMPLER_TEXTADDR_MIRROR_ONCE
} MHW_SAMPLER_TEXTADDR;

typedef struct _MHW_AVS_COEFFICIENT_PARAM
{
    int8_t ZeroXFilterCoefficient[8];
    int8_t ZeroYFilterCoefficient[8];
    int8_t OneXFilterCoefficient[4]; //!< [0..3] maps to filter coefficients [2..5], in actual tabel [0..1] are reserved
    int8_t OneYFilterCoefficient[4]; //!< [0..3] maps to filter coefficients [2..5], in actual tabel [0..1] are reserved
} MHW_AVS_COEFFICIENT_PARAM, *PMHW_AVS_COEFFICIENT_PARAM;

typedef struct _MHW_SAMPLER_AVS_TABLE_PARAM
{
    MHW_AVS_COEFFICIENT_PARAM paMhwAvsCoeffParam[MHW_NUM_HW_POLYPHASE_TABLES];

    // sampler table control
    uint8_t byteTransitionArea8Pixels; //!< only least 3-bits used
    uint8_t byteTransitionArea4Pixels; //!< only least 3-bits used
    uint8_t byteMaxDerivative8Pixels;
    uint8_t byteMaxDerivative4Pixels;
    uint8_t byteDefaultSharpnessLevel;

    bool bEnableRGBAdaptive;
    bool bAdaptiveFilterAllChannels;
    bool bBypassYAdaptiveFiltering;
    bool bBypassXAdaptiveFiltering;

    bool b8TapAdaptiveEnable;
    bool b4TapGY;
    bool b4TapRBUV;

    bool bIsCoeffExtraEnabled;
    MHW_AVS_COEFFICIENT_PARAM paMhwAvsCoeffParamExtra[MHW_NUM_HW_POLYPHASE_EXTRA_TABLES_G9]; // only for gen9+

} MHW_SAMPLER_AVS_TABLE_PARAM, *PMHW_SAMPLER_AVS_TABLE_PARAM;

//!
//! \brief  Sampler States for 8x8 sampler
//!
typedef struct _MHW_SAMPLER_STATE_AVS_PARAM
{
    int16_t                      stateID;

    // STE params
    bool                         bEnableSTDE;           // a.k.a SkinToneTunedIEF
    bool                         b8TapAdaptiveEnable;
    bool                         bSkinDetailFactor;
    bool                         bHdcDwEnable;          // Gen9+
    bool                         bWritebackStandard;    // set Writeback same as Original Sample_8x8

    // IEF params
    bool                         bEnableIEF;
    uint16_t                     wIEFFactor;            // 0 will disable IEF
    uint16_t                     wR3xCoefficient;
    uint16_t                     wR3cCoefficient;
    uint16_t                     wR5xCoefficient;
    uint16_t                     wR5cxCoefficient;
    uint16_t                     wR5cCoefficient;

    // AVS params
    bool                         bEnableAVS;
    bool                         AvsType;          // 0 - Polyphase; 1 - nearest
    bool                         EightTapAFEnable;
    bool                         BypassIEF;        // ignored for BWL, moved to sampler8x8 payload.
    uint16_t                     GainFactor;
    uint8_t                      GlobalNoiseEstm;
    uint8_t                      StrongEdgeThr;
    uint8_t                      WeakEdgeThr;
    uint8_t                      StrongEdgeWght;
    uint8_t                      RegularWght;
    uint8_t                      NonEdgeWght;

    // Additional overrides
    uint16_t AdditionalOverridesUsed;
    uint16_t YSlope2;
    uint16_t S0L;
    uint16_t YSlope1;
    uint16_t S2U;
    uint16_t S1U;

    PMHW_SAMPLER_AVS_TABLE_PARAM pMhwSamplerAvsTableParam; // pointer to AVS scaling 8x8 table params

    int32_t                      iTable8x8_Index;   // Table allocation index (not needed on Gen8+)
    void                         *pTable8x8_Ptr;     // Table data ptr in GSH
    uint32_t                     dwTable8x8_Offset; // Table data offset in GSH
} MHW_SAMPLER_STATE_AVS_PARAM, *PMHW_SAMPLER_STATE_AVS_PARAM;

//!
//! \brief  Structure to handle UNORM sampler states
//!
typedef enum _MHW_SAMPLER_SURFACE_PIXEL_TYPE
{
    MHW_SAMPLER_SURFACE_PIXEL_UINT,
    MHW_SAMPLER_SURFACE_PIXEL_SINT,
    MHW_SAMPLER_SURFACE_PIXEL_OTHER
} MHW_SAMPLER_SURFACE_PIXEL_TYPE;

typedef enum _MHW_GFX3DSTATE_MAPFILTER
{
    MHW_GFX3DSTATE_MAPFILTER_NEAREST        = 0x0,
    MHW_GFX3DSTATE_MAPFILTER_LINEAR         = 0x1,
    MHW_GFX3DSTATE_MAPFILTER_ANISOTROPIC    = 0x2,
    MHW_GFX3DSTATE_MAPFILTER_FLEXIBLE       = 0x3,
    MHW_GFX3DSTATE_MAPFILTER_MONO           = 0x6
} MHW_GFX3DSTATE_MAPFILTER;

typedef enum _MHW_GFX3DSTATE_TEXCOORDMODE
{
    MHW_GFX3DSTATE_TEXCOORDMODE_WRAP            = 0,
    MHW_GFX3DSTATE_TEXCOORDMODE_MIRROR          = 1,
    MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP           = 2,
    MHW_GFX3DSTATE_TEXCOORDMODE_CUBE            = 3,
    MHW_GFX3DSTATE_TEXCOORDMODE_CLAMP_BORDER    = 4,
    MHW_GFX3DSTATE_TEXCOORDMODE_MIRROR_ONCE     = 5
} MHW_GFX3DSTATE_TEXCOORDMODE;

typedef enum _MHW_CHROMAKEY_MODE
{
    MHW_CHROMAKEY_MODE_KILL_ON_ANY_MATCH = 0,
    MHW_CHROMAKEY_MODE_REPLACE_BLACK = 1
} MHW_CHROMAKEY_MODE;

typedef struct _MHW_SAMPLER_STATE_UNORM_PARAM
{
    MHW_SAMPLER_FILTER_MODE      SamplerFilterMode;
    MHW_GFX3DSTATE_MAPFILTER     MagFilter;
    MHW_GFX3DSTATE_MAPFILTER     MinFilter;
    MHW_GFX3DSTATE_TEXCOORDMODE  AddressU;
    MHW_GFX3DSTATE_TEXCOORDMODE  AddressV;
    MHW_GFX3DSTATE_TEXCOORDMODE  AddressW;

    MHW_SAMPLER_SURFACE_PIXEL_TYPE SurfaceFormat;
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

    uint32_t IndirectStateOffset;
    void  *pIndirectState;
    bool bBorderColorIsValid;

    bool bChromaKeyEnable;
    uint32_t ChromaKeyIndex;
    MHW_CHROMAKEY_MODE ChromaKeyMode;
} MHW_SAMPLER_STATE_UNORM_PARAM, *PMHW_SAMPLER_STATE_UNORM_PARAM;

//!
//! \brief  Structure to capture sizes of HW engine commands and structures
//!
typedef struct _MHW_RENDER_STATE_SIZES
{
    uint32_t            dwSizeMediaObjectHeaderCmd;                             // Size of Media Object Header Command
    uint32_t            dwMaxSizeSurfaceState;                                  // Max size of a surface state
    uint32_t            dwSizeSurfaceState;                                     // Size of surface state
    uint32_t            dwSizeSurfaceStateAvs;                                  // Size of AVS surface state
    uint32_t            dwSizeBindingTableState;                                // Size of binding table state entry
    uint32_t            dwSizeSamplerState;                                     // Size of sampler state (unorm)
    uint32_t            dwSizeSamplerIndirectState;                             // Size of sampler indirect state (unorm)
    uint32_t            dwSizeSamplerStateAvs;                                  // Size of sampler state (Avs)
    uint32_t            dwSizeSamplerStateVA;                                   // Size of sampler state (va)
    uint32_t            dwSizeSamplerStateVAConvolve;                           // Size of sampler state (va convolve)
    uint32_t            dwSizeSamplerStateTable8x8;                             // Size of sampler state 8x8 table
    uint32_t            dwSizeSampler8x8Table;                                  // Size of sampler 8x8 table
    uint32_t            dwSizeInterfaceDescriptor;                              // Size of interface descriptor
    uint32_t            dwSizeMediaWalkerBlock;                                 // Size of Media Walker block
} MHW_RENDER_STATE_SIZES, *PMHW_RENDER_STATE_SIZES;

//!
//! \brief  Structure to handle VME Sampler State
//!
typedef struct _MHW_SAMPLER_STATE_VME_PARAM
{
    uint32_t                     *pdwLUTSearchPath;
    uint32_t                     *pdwLUTMbMode;
    uint32_t                     *pdwLUTMv;
} MHW_SAMPLER_STATE_VME_PARAM, *PMHW_SAMPLER_STATE_VME_PARAM;

typedef struct _MHW_SAMPLER_CONVOLVE_COEFF_TABLE
{
    uint16_t wFilterCoeff[16];
} MHW_SAMPLER_CONVOLVE_COEFF_TABLE;

typedef struct _MHW_SAMPLER_STATE_CONVOLVE_PARAM
{
    uint8_t                      ui8ConvolveType;       // 1d, 2d
    uint8_t                      ui8Height;
    uint8_t                      ui8Width;
    uint8_t                      ui8ScaledDownValue;
    uint8_t                      ui8SizeOfTheCoefficient;
    uint8_t                      ui8MSBHeight;
    uint8_t                      ui8MSBWidth;
    bool                         skl_mode;
    MHW_SAMPLER_CONVOLVE_COEFF_TABLE CoeffTable[62];
} MHW_SAMPLER_STATE_CONVOLVE_PARAM, *PMHW_SAMPLER_STATE_CONVOLVE_PARAM;

typedef struct _MHW_SAMPLER_8x8_MISC_STATE {
    uint8_t byteHeight;
    uint8_t byteWidth;
    uint16_t wRow[15];
} MHW_SAMPLER_8x8_MISC_STATE, *PMHW_SAMPLER_8x8_MISC_STATE;

//!
//! \brief  Structure to handle Sampler State
//!
typedef struct _MHW_SAMPLER_STATE_PARAM
{
    bool                        bInUse;
    PMHW_KERNEL_STATE           pKernelState;
    MHW_SAMPLER_TYPE            SamplerType;
    MHW_SAMPLER_ELEMENT_TYPE    ElementType;
    union
    {
        MHW_SAMPLER_STATE_UNORM_PARAM     Unorm;
        MHW_SAMPLER_STATE_AVS_PARAM       Avs;
        MHW_SAMPLER_STATE_VME_PARAM       Vme;
        MHW_SAMPLER_STATE_CONVOLVE_PARAM  Convolve;
        MHW_SAMPLER_8x8_MISC_STATE        Misc;
    };
} MHW_SAMPLER_STATE_PARAM, *PMHW_SAMPLER_STATE_PARAM;

typedef struct _MHW_SURFACE_STATE_SEND_PARAMS {
    uint8_t                 *pIndirectStateBase;
    uint8_t                 *pSurfaceStateSource;
    uint8_t                 *pSurfaceToken;
    int32_t                 iIndirectStateBase;
    int32_t                 iSurfaceStateOffset;
    bool                    bNeedNullPatch;
} MHW_SURFACE_STATE_SEND_PARAMS, *PMHW_SURFACE_STATE_SEND_PARAMS;

struct MHW_STATE_HEAP_SETTINGS
{
    MHW_STATE_HEAP_SETTINGS() {}

    virtual ~MHW_STATE_HEAP_SETTINGS() {}

    uint32_t        dwIshSize = 0;          //!< Initial size of ISH
    uint32_t        dwDshSize = 0;          //!< Initial size of DSH
    uint32_t        dwIshIncrement = 0;     //!< ISH increment step
    uint32_t        dwDshIncrement = 0;     //!< DSH increment step
    uint32_t        dwIshMaxSize = 0;       //!< ISH max size
    uint32_t        dwDshMaxSize = 0;       //!< DSH max size

    bool            m_keepIshLocked = false;       //!< Keep ISH locked always
    bool            m_keepDshLocked = false;       //!< Keep DSH locked always

    HeapManager::Behavior m_ishBehavior = HeapManager::Behavior::wait;    //!< ISH behavior
    HeapManager::Behavior m_dshBehavior = HeapManager::Behavior::wait;    //!< DSH behavior

    uint32_t        dwNumSyncTags = 0; //!< to be removed with old interfaces
};

typedef struct _MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS
{
    int32_t                      *piSizes;               //<! [in] array of block sizes to allocate
    int32_t                      iCount;                //<! [in] number of blocks to allocate
    uint32_t                     dwAlignment;           //<! [in] block alignment
    bool                         bHeapAffinity;         //<! [in] true if all blocks must be allocated in the same heap; false otherwize
    PMHW_STATE_HEAP              pHeapAffinity;         //<! [in] Select a specific heap to allocate (nullptr if don't care)
    uint32_t                     dwScratchSpace;        //<! [in/out] Scratch space requested, scratch space allocated in same heap as the block
    PMHW_STATE_HEAP_MEMORY_BLOCK pScratchSpace;         //<! [out] Pointer to scratch space block used - needed for command buffer setup
    bool                         bZeroAssignedMem;      //<! [in] Zero memory blocks after allocation
    bool                         bStatic;               //<! [in] Block allocations are flaged as static
    bool                         bGrow;                 //<! [in] Allow state heap to grow in order to satisfy the request
} MHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS, *PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS;

typedef MOS_STATUS ( *pfnAddResourceToCmd) (PMOS_INTERFACE , PMOS_COMMAND_BUFFER ,PMHW_RESOURCE_PARAMS);

class XMHW_STATE_HEAP_INTERFACE
{
public:
    static const uint32_t m_mhwBindingTableOffsetShift  = 6;
    static const uint32_t m_mhwBindingTableSurfaceShift = 6;
    static const uint32_t m_mhwGenericOffsetShift       = 6;
    static const uint32_t m_mhwBindingTableOffsetAlignment = (1 << m_mhwBindingTableOffsetShift);
    static const uint32_t m_mhwNumBindingTableEntryOffset  = (m_mhwBindingTableOffsetAlignment/4);

protected:
    HeapManager m_ishManager;
    HeapManager m_dshManager;
    std::vector<MemoryBlock> m_blocks;
    std::vector<uint32_t> m_blockSizes;

private:
    MEDIA_WA_TABLE          *m_pWaTable;
    MHW_STATE_HEAP_SETTINGS m_StateHeapSettings;

    // old heap management
    MOS_RESOURCE            m_resCmdBufIdGlobal;
    uint32_t                *m_pdwCmdBufIdGlobal;
    uint32_t                m_dwCurrCmdBufId;
    PMHW_SYNC_TAG           m_pSyncTags;
    uint32_t                m_dwCurrSyncTag;
    uint32_t                m_dwInvalidSyncTagId; //!< Passed in at creation by the client
    bool                    m_bRegisteredBBCompleteNotifyEvent;
    PMHW_STATE_HEAP         m_pInstructionStateHeaps;
    uint32_t                m_dwNumIsh;
    uint32_t                m_dwNumDsh;
    PMHW_STATE_HEAP         m_pDynamicStateHeaps;
    int8_t                  m_bDynamicMode;     //!< To be deprecated, 0 - RenderHal handles, 1 - MDF heap management, 2 - generic hep
    PMHW_BLOCK_MANAGER      m_pIshBlockManager; //!< ISH block management object
    PMHW_BLOCK_MANAGER      m_pDshBlockManager; //!< DSH block management object

public:
    PMOS_INTERFACE          m_pOsInterface;
    uint16_t                m_wIdAlignment;
    uint16_t                m_wBtIdxAlignment;
    uint16_t                m_wCurbeAlignment;
    uint16_t                m_wSizeOfCmdSamplerState;
    uint32_t                m_dwMaxSurfaceStateSize;
    pfnAddResourceToCmd     m_pfnAddResourceToCmd;
    MHW_STATE_HEAP          m_SurfaceStateHeap; //!< Simulated SSH with MHW_STATE_HEAP.
    uint16_t                m_wSizeOfCmdInterfaceDescriptorData;
    MHW_RENDER_STATE_SIZES  m_HwSizes;

public:
    //!
    //! \brief Internal to MHW
    //!

    //!
    //! \brief    Constructor of the MI StateHeap interface
    //! \details  Internal MHW function to initialize all function pointers and some parameters
    //! \param    [in] pCpInterface
    //!           CP interface, must be valid
    //! \param    [in] pOsInterface
    //!           OS interface, must be valid
    //!
    XMHW_STATE_HEAP_INTERFACE(PMOS_INTERFACE pInputOSInterface, int8_t bDynamicMode=0);

    XMHW_STATE_HEAP_INTERFACE(const XMHW_STATE_HEAP_INTERFACE&) = delete;

    XMHW_STATE_HEAP_INTERFACE& operator=(const XMHW_STATE_HEAP_INTERFACE&) = delete;

    virtual ~XMHW_STATE_HEAP_INTERFACE();

    PMHW_STATE_HEAP GetDSHPointer(){ return   m_pDynamicStateHeaps; };

    PMHW_STATE_HEAP GetISHPointer(){ return   m_pInstructionStateHeaps;};

    uint32_t GetNumDsh(){return m_dwNumDsh;};

    uint32_t GetNumIsh(){return m_dwNumIsh;};

    PMOS_RESOURCE  GetResCmdBufIdGlobal(){return &m_resCmdBufIdGlobal;};

    PMHW_SYNC_TAG  GetSycnTags(){return m_pSyncTags;};

    uint16_t GetIdAlignment(){return m_wIdAlignment;};

    uint16_t GetSizeofCmdSampleState(){return m_wSizeOfCmdSamplerState;};

    uint16_t GetSizeofCmdInterfaceDescriptorData(){ return m_wSizeOfCmdInterfaceDescriptorData;};

    uint16_t GetCurbeAlignment(){ return m_wCurbeAlignment;};

    uint16_t GetBtIdxAlignment(){ return m_wBtIdxAlignment;};

    uint32_t GetCurrCmdBufId(){ return m_dwCurrCmdBufId;};

    uint32_t GetCmdBufIdGlobal(){ return *m_pdwCmdBufIdGlobal;};

    uint32_t *GetCmdBufIdGlobalPointer(){ return m_pdwCmdBufIdGlobal; };

    PMHW_RENDER_STATE_SIZES GetHwSizesPointer() { return & m_HwSizes;};

    uint32_t GetSizeofSamplerStateAvs() { return m_HwSizes.dwSizeSamplerStateAvs;};

    //!
    //! \brief    Initializes the MI StateHeap interface
    //! \details  Internal MHW function to initialize all function pointers and some parameters
    //!           Assumes that the caller has checked pointer validity and whether or not an
    //!           addressing method has been selected in the OS interface (bUsesGfxAddress or
    //!           bUsesPatchList).
    //! \param    [in] StateHeapSettings
    //!           StateHeap setting passed from caller
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    MOS_STATUS InitializeInterface(MHW_STATE_HEAP_SETTINGS     StateHeapSettings);

    //!
    //! \brief    Assigns space in a state heap to a kernel state
    //! \details  Client facing function to assign as space in a state heap a kernel state;
    //!           if no space is available, a clean up is attempted 
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] The state heap type requested (ISH/DSH)
    //! \param    PMHW_KERNEL_STATE pKernelState
    //!           [in] Kernel state to which a state heap space will be assigned
    //! \param    uint32_t dwSpaceRequested
    //!           [in] The amount of space requested from the state heap
    //! \param    bool bStatic
    //!           [in] Whether or not the space requested is static
    //! \param    bool bZeroAssignedMem
    //!           [in] Whether or not acquired memory should be zeroed
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, MOS_STATUS_CLIENT_AR_NO_SPACE if no space
    //!           is available but it is possible for the client to wait, else fail reason
    //!
    MOS_STATUS AssignSpaceInStateHeap(
        MHW_STATE_HEAP_TYPE         StateHeapType,
        PMHW_KERNEL_STATE           pKernelState,
        uint32_t                    dwSpaceRequested,
        bool                        bStatic,
        bool                        bZeroAssignedMem);

    //!
    //! \brief    Submits all non-static blocks in kernel state
    //! \param    [in] pKernelState
    //!           Kernel state containing all memory blocks to submit
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS SubmitBlocks(PMHW_KERNEL_STATE pKernelState);

    //!
    //! \brief    Locks requested state heap
    //! \details  Client facing function to lock a state heap
    //! \param    PMHW_STATE_HEAP pStateHeap
    //!           [in] The state heap to be locked
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS LockStateHeap(
        PMHW_STATE_HEAP             pStateHeap);

    //!
    //! \brief    Unlocks requested state heap
    //! \details  Client facing function to unlock a state heap
    //! \param    PMHW_STATE_HEAP pStateHeap
    //!           [in] The state heap to be locked
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success
    //!
    MOS_STATUS UnLockStateHeap(
        PMHW_STATE_HEAP             pStateHeap);

    //!
    //! \brief    Allocates a state heap of the requested type
    //! \details  Client facing function to extend a state heap of the requested time, which
    //!           involves allocating state heap and added it to the state heap liked list.
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] The state heap type requested (ISH/DSH)
    //! \param    uint32_t dwSizeRequested
    //!           [in] The size of the state heap
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS ExtendStateHeap(
        MHW_STATE_HEAP_TYPE         StateHeapType,
        uint32_t                    dwSizeRequested);

    //!
    //! \brief    Update CmdBufIdGlobal
    //! \details  Client facing function to update CmdBufIdGlobal
    //!           reset current offset to zero
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS UpdateGlobalCmdBufId();

    //!
    //! \brief    Set command buffer status pointer
    //! \details  Client facing function to set command buffer status pointer
    //! \param    void  *pvCmdBufStatus
    //!           [in] command buffer status pointer
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS SetCmdBufStatusPtr(void *pvCmdBufStatus);

    //!
    //! \brief    Calculate the space needed in the SSH
    //! \details  Client facing function to calculate the space needed in the SSH
    //!           given the number of binding table entries
    //! \param    uint32_t dwBtEntriesRequested
    //!           [in] Binding table entries requested in the SSH
    //! \param    uint32_t *pdwSshSize
    //!           [out] The size needed in the SSH
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS CalculateSshAndBtSizesRequested(
        uint32_t                    dwBtEntriesRequested,
        uint32_t                    *pdwSshSize,
        uint32_t                    *pdwBtSize);

    //!
    //! \brief    Request SSH space for a command buffer.
    //! \details  Client facing function to request SSH space for a command buffer, if not enough
    //!           space is available, more will be requested.
    //! \param    uint32_t dwBtEntriesRequested
    //!           [in] Binding table entries requested in the SSH
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS RequestSshSpaceForCmdBuf(
        uint32_t                    dwBtEntriesRequested);

    //!
    //! \brief    Calculates the amount of space needed
    //! \details  Tells if there is enough space available in heap to load an array of blocks
    //            Returns how much is missing (needs to be freed, 0 if none)
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] State heap type (DSH/ISH)
    //! \param    PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams
    //!           [in] Dynamic state heap parameters
    //! \return   uint32_t
    //!           Amount of space needed in bytes
    //!
    uint32_t CalculateSpaceNeededDyn(
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams);

    //Virtual Interfaces

    //!
    //! \brief    Adds INTERFACE_DESCRIPTOR command(s) to the DSH
    //! \details  Client facing function to add INTERFACE_DESCRIPTOR(s) to the DSH
    //! \param    uint32_t dwNumIdsToSet
    //!           [in] The number of pParams
    //! \param    PMHW_INTERFACE_DESCRIPTOR_PARAMS pParams
    //!           [in] Parameters used to set the INTERFACE_DESCRIPTOR(s)
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetInterfaceDescriptor(
        uint32_t                            dwNumIdsToSet,
        PMHW_INTERFACE_DESCRIPTOR_PARAMS    pParams) = 0;

    //!
    //! \brief    Setup Media Interface Descriptor Entry in DSH
    //! \details  Setup Single Media Interface Descriptor Entry
    //! \param    PMHW_ID_ENTRY_PARAMS pParams
    //!           [in] Interface Descriptor Parameters
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] Pointer to Command Buffer
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS SetInterfaceDescriptorEntry(
        PMHW_ID_ENTRY_PARAMS                pParams) = 0;

    //!
    //! \brief    Adds media interface descriptor data to dynamic GSH
    //! \param    PMHW_ID_ENTRY_PARAMS pParams
    //!           [in] Interface descriptor parameters
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS AddInterfaceDescriptorData(
        PMHW_ID_ENTRY_PARAMS                pParams) = 0;

    //!
    //! \brief    Adds a binding table to the SSH
    //! \details  Client facing function to add binding table to SSH
    //! \param    PMHW_KERNEL_STATE pKernelState
    //!           [in] Kernel state to construct the binding table for
    //! \param    void  *pvHwInterface
    //!           [in] Temporary input parameter until SSH setup occurs in MHW
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetBindingTable(
        PMHW_KERNEL_STATE                   pKernelState)= 0;

    //!
    //! \brief    Sets binding table entry in SSH
    //! \details  TO BE REMOVED! Client facing function to add binding table to SSH
    //! \param    PMHW_BINDING_TABLE_PARAMS pParams
    //!           [in] Surface binding parameters
    //!
    virtual MOS_STATUS SetBindingTableEntry(
        PMHW_BINDING_TABLE_PARAMS        pParams) = 0;

    //!
    //! \brief    Sends binding table entry to Command Buffer (indirect state)
    //! \details  TO BE REMOVED! Sends binding table entry to indirect state heap in Cmd buffer,
    //!           retrieving associated surface state offset and index
    //! \param    PMHW_BINDING_TABLE_PARAMS pParams
    //!           [in] Surface binding parameters - returns surface state pointer and index
    //!
    virtual MOS_STATUS SendBindingTableEntry(
        PMHW_BINDING_TABLE_SEND_PARAMS   pParams) = 0;

    //!
    //! \brief    set surface state entry 
    //! \details  TO BE REMOVED! set surface state entry
    //! \param    PMHW_BINDING_TABLE_PARAMS pParams
    //!           [in] Surface state parameters
    //!
    virtual MOS_STATUS SetSurfaceStateEntry(
        PMHW_SURFACE_STATE_PARAMS   pParams) = 0;

    //!
    //! \brief    Set surface state in ssh
    //! \details  Set sampler state in ssh
    //! \param    PMOS_COMMAND_BUFFER pCmdBuffer
    //!           [in] command buffer pointer
    //! \param    uint32_t dwNumSurfaceStatesToSet
    //!           [in] number of surface states need to set
    //! \param    PMHW_RCS_SURFACE_PARAMS pParams
    //!           [in] render surface state parameters
    //!
    virtual MOS_STATUS SetSurfaceState(
        PMHW_KERNEL_STATE           pKernelState,
        PMOS_COMMAND_BUFFER         pCmdBuffer,
        uint32_t                    dwNumSurfaceStatesToSet,
        PMHW_RCS_SURFACE_PARAMS     pParams) = 0;

    //!
    //! \brief    Set sampler state
    //! \details  Set sampler state
    //! \param    PMHW_SAMPLER_STATE_PARAM pParams
    //!           [in] Sampler state parameters
    //!
    virtual MOS_STATUS SetSamplerState(
        void                        *pSampler,
        PMHW_SAMPLER_STATE_PARAM    pParam) = 0;

    //!
    //! \brief    Adds sampler state data to dynamic GSH
    //! \param    uint32_t samplerOffset
    //!           [in] sampler offset
    //! \param    MemoryBlock memoryBlock
    //!           [in,out] Pointer to memory block
    //! \param    PMHW_SAMPLER_STATE_PARAM pParam
    //!           [in] sampler state parameters
    //! \return   MOS_STATUS
    //!
    virtual MOS_STATUS AddSamplerStateData(
        uint32_t                    samplerOffset,
        MemoryBlock                 *memoryBlock,
        PMHW_SAMPLER_STATE_PARAM    pParam) = 0;

    //!
    //! \brief    Initialize sampler states
    //! \details  Initialize sampler states
    //! \param    void  *pSamplerStates
    //!           [in] Pointer to sampler states to reset
    //! \param    int32_t iSamplers
    //!           [in] Number of sampler entries to reset
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS InitSamplerStates(
        void                        *pSamplerStates,
        int32_t                     iSamplers) = 0;

    //!
    //! \brief    Init HwSizes
    //! \details  Init HwSizes 
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    virtual MOS_STATUS InitHwSizes() = 0;

    //!
    //! \brief    Releases the dynamic state heap
    //! \details  Releases the dynamic state heap. If all blocks
    //!           are completed, free the resource. If there are some blocks still in
    //!           the submitted state, then mark blocks for deletion. This function will be
    //!           called again to free the resource when the last block is released. 
    //! \param    PMHW_STATE_HEAP pStateHeap
    //!           [in] Pointer to the state heap to be released
    //! \return   MOS_STATUS
    //!           SUCCESS    if state heap was either marked for deletion or actually freed
    //!
    MOS_STATUS ReleaseStateHeapDyn(PMHW_STATE_HEAP pStateHeap);

    //!
    //! \brief    Allocates a dynamic block
    //! \details  Allocates either a single dynamic block or multiple blocks in ISH/DSH
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] State heap type (DSH/ISH)
    //! \param    PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams
    //!           [in] Dynamic state heap parameters
    //! \return   PMHW_STATE_HEAP_MEMORY_BLOCK
    //!           Pointer to the allocated memory block(s)
    //!
    PMHW_STATE_HEAP_MEMORY_BLOCK AllocateDynamicBlockDyn(
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams);

    //!
    //! \brief    Submits a dynamic block
    //! \details  Submits a dynamic block. Detaches the block from current list and adds to to
    //!           submitted list. Updates block with provided sync tag.
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] State heap type (DSH/ISH)
    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock
    //!           [in] Pointer to memory block to be submitted
    //! \param    uint32_t dwSyncTag
    //!           [in] Sync Tag
    //! \return   MOS_STATUS
    //!           SUCCESS    if submission was successful
    //!
    MOS_STATUS SubmitDynamicBlockDyn(
            MHW_STATE_HEAP_TYPE                  StateHeapType,
            PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock,
            const FrameTrackerTokenFlat          *trackerToken);

    //!
    //! \brief    Frees a dynamic block
    //! \details  Detaches the block from current list if block has completed (based on provided dwSyncTag)
    //!           and adds it to deleted list. Otherwise, marks for deletion upon completion.
    //!           submitted list. Updates block with provided sync tag.
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] State heap type (DSH/ISH)
    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK pBlock
    //!           [in] Pointer to memory block to be freed
    //! \return   MOS_STATUS
    //!           SUCCESS    if operation was successful
    //!
    MOS_STATUS FreeDynamicBlockDyn(
            MHW_STATE_HEAP_TYPE                  StateHeapType,
            PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock);

    //!
    //! \brief    Refresh the dynamic heap
    //! \details  Updates block states based on last executed tag
    //!           submitted unlocked blocks are released;
    //!           move to allocated
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] State heap type (DSH/ISH)
    //! \param    uint32_t dwSyncTag
    //!           [in] Sync Tag
    //! \return   MOS_STATUS
    //!           SUCCESS    if operation was successful
    //!
    MOS_STATUS RefreshDynamicHeapDyn (
        MHW_STATE_HEAP_TYPE         StateHeapType);

private:

    //!
    //! \brief    Insert a node into a memory block linked list
    //! \details  MHW private function which maintains the memory block linked list
    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK pStartNode
    //!           [in] The memory block from which to start the insertion
    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK pNodeToAdd
    //!           [in] The memory block  to insert
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    MOS_STATUS InsertLinkedList(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pStartNode,
        PMHW_STATE_HEAP_MEMORY_BLOCK    pNodeToAdd);

    //!
    //! \brief    Allocate and initialize a memory block based on input parameters
    //! \details  MHW private function which creates memory blocks
    //! \param    PMHW_STATE_HEAP pStateHeap
    //!           [in] The state heap which the memory block belongs to
    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK ppMemoryBlock
    //!           [in/out] The node to insert.
    //! \param    uint32_t dwRequestedSize
    //!           [in] The size of the memory that the memory block references.
    //! \param    bool bStatic
    //!           [in] If the memory block is static
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    MOS_STATUS InitMemoryBlock(
        PMHW_STATE_HEAP                 pStateHeap,
        PMHW_STATE_HEAP_MEMORY_BLOCK    *ppMemoryBlock,
        uint32_t                        dwRequestedSize,
        bool                            bStatic);

    //!
    //! \brief    Inserts a new memory block into an existing available memory block
    //! \details  MHW private function to insert new memory blocks into available memory blocks
    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK pMemoryBlockFree
    //!           [in] Available memory block
    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK pMemoryBlockToAdd
    //!           [in] New memory block to be inserted
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    MOS_STATUS InsertMemoryBlock(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlockFree,
        PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlockToAdd);

    //!
    //! \brief    Returns the space of the memory block to the state heap
    //! \details  MHW private function to return the memory block space to the state heap
    //!           by marking it as available and merging it with adjacent memory blocks

    //! \param    PMHW_STATE_HEAP_MEMORY_BLOCK pMemoryBlock
    //!           [in] Memory block to return to the state heap
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success else fail reason
    //!
    MOS_STATUS ReturnSpaceMemoryBlock(
        PMHW_STATE_HEAP_MEMORY_BLOCK    pMemoryBlock);

    //Interfaces different cross static and dynmaic mode

    //!
    //! \brief    Extends the dynamic state heap
    //! \details  Allocates a dynamic state heap (ISH/DSH) with requested size
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] State heap type (i.e. ISH/DSH)
    //! \param    uint32_t dwSizeRequested
    //!           [in] Size of requested state heap
    //! \return   MOS_STATUS
    //!           SUCCESS    if state heap was successfully allocated
    //!
    MOS_STATUS ExtendStateHeapDyn(
        MHW_STATE_HEAP_TYPE         StateHeapType,
        uint32_t                    dwSizeRequested);

    //!
    //! \brief    Extends the dynamic state heap
    //! \details  Allocates a dynamic state heap (ISH/DSH) with requested size
    //! \param    MHW_STATE_HEAP_TYPE StateHeapType
    //!           [in] State heap type (i.e. ISH/DSH)
    //! \param    uint32_t dwSizeRequested
    //!           [in] Size of requested state heap
    //! \return   MOS_STATUS
    //!           SUCCESS    if state heap was successfully allocated
    //!
    MOS_STATUS ExtendStateHeapSta(
        MHW_STATE_HEAP_TYPE         StateHeapType,
        uint32_t                    dwSizeRequested);

};

struct _MHW_STATE_HEAP_INTERFACE
{

    XMHW_STATE_HEAP_INTERFACE               *pStateHeapInterface;

    //!
    //! \brief Internal to MHW
    //!

    MOS_STATUS (*pfnCreate) (
        PMHW_STATE_HEAP_INTERFACE   *ppStateHeapInterface,
        MHW_STATE_HEAP_SETTINGS     StateHeapSettings);

    MOS_STATUS (*pfnDestroy) (
        PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface);

    //!
    //! \brief Primarily a state heap interface private function to be
    //!        called as a pair in any state heap interface function which
    //!        accesses the graphics resource like AddDataToStateHeap.
    //!
    MOS_STATUS (*pfnLockStateHeap) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        PMHW_STATE_HEAP                    pStateHeap);

    MOS_STATUS (*pfnUnlockStateHeap) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        PMHW_STATE_HEAP                    pStateHeap);

    //!
    //! \brief Client facing functions
    //!
    MOS_STATUS (*pfnAssignSpaceInStateHeap) (
        PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
        MHW_STATE_HEAP_TYPE         StateHeapType,
        PMHW_KERNEL_STATE           pKernelState,
        uint32_t                    dwSpaceRequested,
        bool                        bStatic,
        bool                        bZeroAssignedMem);

    MOS_STATUS(*pfnSubmitBlocks) (
        PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
        PMHW_KERNEL_STATE           pKernelState);

    MOS_STATUS (*pfnExtendStateHeap) (
        PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
        MHW_STATE_HEAP_TYPE         StateHeapType,
        uint32_t                    dwSizeRequested);

    MOS_STATUS (*pfnSetInterfaceDescriptor) (
        PMHW_STATE_HEAP_INTERFACE           pCommonStateHeapInterface,
        uint32_t                            dwNumIdsToSet,
        PMHW_INTERFACE_DESCRIPTOR_PARAMS    pParams);

    MOS_STATUS (*pfnSetInterfaceDescriptorEntry) (
        PMHW_STATE_HEAP_INTERFACE           pCommonStateHeapInterface,
        PMHW_ID_ENTRY_PARAMS                pParams);

    MOS_STATUS(*pfnSetBindingTable) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        PMHW_KERNEL_STATE                  pKernelState);

    MOS_STATUS (*pfnSetSurfaceState) (
        PMHW_STATE_HEAP_INTERFACE           pCommonStateHeapInterface,
        PMHW_KERNEL_STATE                   pKernelState,
        PMOS_COMMAND_BUFFER                 pCmdBuffer,
        uint32_t                            dwNumSurfaceStatesToSet,
        PMHW_RCS_SURFACE_PARAMS             pParams);

    MOS_STATUS (*pfnSetSurfaceStateBuffer) (
        PMHW_STATE_HEAP_INTERFACE           pCommonStateHeapInterface,
        PMHW_RCS_SURFACE_PARAMS             pParams,
        void                                *pSurfaceState);

    MOS_STATUS (*pfnSetBindingTableEntry) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        PMHW_BINDING_TABLE_PARAMS          pParams);

    MOS_STATUS (*pfnSendBindingTableEntry) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        PMHW_BINDING_TABLE_SEND_PARAMS     pParams);

    MOS_STATUS (* pfnSetSurfaceStateToken) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        PMHW_SURFACE_TOKEN_PARAMS          pParams,
        void                               *pSurfaceStateToken);

    MOS_STATUS (*pfnSetSurfaceStateEntry) (
        PMHW_STATE_HEAP_INTERFACE           pCommonStateHeapInterface,
        PMHW_SURFACE_STATE_PARAMS           pParams);

    MOS_STATUS (*pfnSendSurfaceStateEntry) (
        PMHW_STATE_HEAP_INTERFACE           pCommonStateHeapInterface,
        PMOS_COMMAND_BUFFER                 pCmdBuffer,
        PMHW_SURFACE_STATE_SEND_PARAMS      pParams);

    //!
    //! \brief May only operate on the primary state heap.
    //!
    MOS_STATUS (*pfnCompactStateHeap) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        MHW_STATE_HEAP_TYPE                StateHeapType);

    //!
    //! \brief Must be called by the client directly to provide
    //!        pointer for command buffer status for tracking purposes.
    //!
    MOS_STATUS(*pfnSetCmdBufStatusPtr) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        void                               *pvCmdBufStatus);

    //!
    //! \brief Must be called by the client directly after a command
    //!        buffer containing a kernel workload is submitted.
    //!
    MOS_STATUS(*pfnUpdateGlobalCmdBufId) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface);

    //!
    //! \brief Must be called by the client before the client gets a
    //!        command buffer for a kernel workload from MOS. This function
    //!        will resize the SSH the current size is < dwBtEntriesRequested,
    //!        otherwise it does nothing. In the future when SSHes are 
    //!        managed entirely in UMD this function may be deprecated and 
    //!        the SSH may be treated like the
    //!        other state heaps.
    //!
    MOS_STATUS (*pfnRequestSshSpaceForCmdBuf) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        uint32_t                           dwBtEntriesRequested);

    MOS_STATUS (*pfnCalculateSshAndBtSizesRequested) (
        PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
        uint32_t                    dwBtEntriesRequested,
        uint32_t                    *pdwSshSize,
        uint32_t                    *pdwBtSize);

    MOS_STATUS(* pfnInitSamplerStates) (
        PMHW_STATE_HEAP_INTERFACE   pCommonStateHeapInterface,
        void                        *pSampler,
        int32_t                     iSamplers);

    MOS_STATUS (* pfnSetSamplerState) (
        PMHW_STATE_HEAP_INTERFACE          pCommonStateHeapInterface,
        void                               *pSampler,
        PMHW_SAMPLER_STATE_PARAM           pParams);

    //Interfaces in dynamic mode
    uint32_t (*pfnCalculateDynamicSpaceNeeded) (
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams);

    PMHW_STATE_HEAP_MEMORY_BLOCK (*pfnAllocateDynamicBlock) (
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_DYNAMIC_ALLOC_PARAMS pParams);

    MOS_STATUS (*pfnSubmitDynamicBlock) (
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock,
        FrameTrackerTokenFlat                *trackerToken);

    MOS_STATUS (*pfnFreeDynamicBlock) (
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType,
        PMHW_STATE_HEAP_MEMORY_BLOCK         pBlock);

    MOS_STATUS (*pfnRefreshDynamicHeap) (
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        MHW_STATE_HEAP_TYPE                  StateHeapType);

    MOS_STATUS (*pfnReleaseStateHeap) (
        PMHW_STATE_HEAP_INTERFACE            pStateHeapInterface,
        PMHW_STATE_HEAP                      pStateHeap);

};

MOS_STATUS Mhw_StateHeapInterface_InitInterface(
    PMHW_STATE_HEAP_INTERFACE   *ppCommonStateHeapInterface,
    PMOS_INTERFACE               pOsInterface,
    uint8_t                      bDynamicMode
);

extern const uint8_t g_cMhw_VDirection[MHW_NUM_FRAME_FIELD_TYPES];

#endif // __MHW_STATE_HEAP_H__
