/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_render.h
//! \brief    MHW interface for constructing commands for the render engine
//! \details  Impelements the functionalities common across all platforms for MHW_RENDER
//!

#ifndef __MHW_RENDER_H__
#define __MHW_RENDER_H__

#include "mos_os.h"
#include "mhw_utilities.h"
#include "mhw_state_heap.h"
#include "mhw_mi.h"


#define MHW_RENDER_ENGINE_SSH_SURFACES_PER_BT_MAX           256
#define MHW_RENDER_ENGINE_SAMPLERS_MAX                      16
#define MHW_RENDER_ENGINE_SAMPLERS_AVS_MAX                  8
#define MHW_RENDER_ENGINE_MEDIA_PALOAD_SIZE_MAX             512
#define MHW_RENDER_ENGINE_URB_SIZE_MAX                      2048
#define MHW_RENDER_ENGINE_URB_ENTRIES_MAX                   128
#define MHW_RENDER_ENGINE_INTERFACE_DESCRIPTOR_ENTRIES_MAX  64
#define MHW_RENDER_ENGINE_EU_INDEX_MAX                      12
#define MHW_RENDER_ENGINE_SIZE_REGISTERS_PER_THREAD         0x1800

#define MHW_MAX_DEPENDENCY_COUNT                    8

typedef struct _MHW_RENDER_ENGINE_L3_CACHE_SETTINGS
{
    uint32_t   dwCntlReg  = 0;
    uint32_t   dwCntlReg2 = 0;
    uint32_t   dwCntlReg3 = 0;
    uint32_t   dwSqcReg1  = 0;
    uint32_t   dwSqcReg4  = 0;
    uint32_t   dwLra1Reg  = 0;
    virtual ~_MHW_RENDER_ENGINE_L3_CACHE_SETTINGS() {}
} MHW_RENDER_ENGINE_L3_CACHE_SETTINGS, *PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS;

typedef struct _MHW_RENDER_ENGINE_L3_CACHE_CONFIG
{
    uint32_t   dwL3CacheCntlReg_Register  = 0;
    uint32_t   dwL3CacheCntlReg_Setting   = 0;
    uint32_t   dwL3CacheCntlReg2_Register = 0;
    uint32_t   dwL3CacheCntlReg2_Setting  = 0;
    uint32_t   dwL3CacheCntlReg3_Register = 0;
    uint32_t   dwL3CacheCntlReg3_Setting  = 0;
    uint32_t   dwL3CacheSqcReg1_Register  = 0;
    uint32_t   dwL3CacheSqcReg1_Setting   = 0;
    uint32_t   dwL3CacheSqcReg4_Register  = 0;
    uint32_t   dwL3CacheSqcReg4_Setting   = 0;
    uint32_t   dwL3LRA1Reg_Register       = 0;
    uint32_t   dwL3LRA1Reg_Setting        = 0;
    bool       bL3CachingEnabled          = false;
    bool       bL3LRA1Reset               = false;
} MHW_RENDER_ENGINE_L3_CACHE_CONFIG, *PMHW_RENDER_ENGINE_L3_CACHE_CONFIG;

typedef enum _MHW_RENDER_ENGINE_ADDRESS_SHIFT
{
    MHW_RENDER_ENGINE_STATE_BASE_ADDRESS_SHIFT  = 12
} MHW_RENDER_ENGINE_ADDRESS_SHIFT;

typedef enum _MHW_VFE_SLICE_DISABLE
{
    MHW_VFE_SLICE_ALL = 0,
    MHW_VFE_SLICE0_SUBSLICE_ALL,
    MHW_VFE_SLICE0_SUBSLICE0
} MHW_VFE_SLICE_DISABLE;

typedef enum _MHW_WALKER_MODE
{
    MHW_WALKER_MODE_NOT_SET  = -1,
    MHW_WALKER_MODE_DISABLED = 0,
    MHW_WALKER_MODE_SINGLE  = 1,    // dual = 0, repel = 1
    MHW_WALKER_MODE_DUAL    = 2,    // dual = 1, repel = 0)
    MHW_WALKER_MODE_TRI     = 3,    // applies in BDW GT2 which has 1 slice and 3 sampler/VME per slice
    MHW_WALKER_MODE_QUAD    = 4,    // applies in HSW GT3 which has 2 slices and 2 sampler/VME per slice
    MHW_WALKER_MODE_HEX     = 6,    // applies in BDW GT2 which has 2 slices and 3 sampler/VME per slice
    MHW_WALKER_MODE_OCT     = 8     // may apply in future Gen media architectures
} MHW_WALKER_MODE;

//!
//! \brief  Structure to capture HW capabilities
//!
typedef struct _MHW_RENDER_ENGINE_CAPS
{
    uint32_t               dwMaxUnormSamplers;                                     // Max UNORM Sampler States supported
    uint32_t               dwMaxAVSSamplers;                                       // Max AVS Sampler States supported
    uint32_t               dwMaxBTIndex;                                           // Max Binding Table index per Binding Table
    uint32_t               dwMaxThreads;                                           // Max Threads supported
    uint32_t               dwMaxMediaPayloadSize;                                  // Max Media payload size
    uint32_t               dwMaxURBSize;                                           // Max URB Size
    uint32_t               dwMaxURBEntries;                                        // Max URB Entries
    uint32_t               dwMaxURBEntryAllocationSize;                            // Max URB Entry Allocation Size
    uint32_t               dwMaxCURBEAllocationSize;
    uint32_t               dwMaxInterfaceDescriptorEntries;                        // Max Interface Descriptor Entries
    uint32_t               dwMaxSubslice;                                          // Max number of subslice
    uint32_t               dwMaxEUIndex;                                           // Max EU index (sometimes != number of EU)
    uint32_t               dwNumThreadsPerEU;                                      // Num threads per EU
    uint32_t               dwSizeRegistersPerThread;                               // Size of all registers per thread (for ASM Debug)
} MHW_RENDER_ENGINE_CAPS, *PMHW_RENDER_ENGINE_CAPS;

typedef struct _MHW_STATE_BASE_ADDR_PARAMS
{
    PMOS_RESOURCE           presGeneralState;
    uint32_t                dwGeneralStateSize;
    PMOS_RESOURCE           presDynamicState;
    uint32_t                dwDynamicStateSize;
    bool                    bDynamicStateRenderTarget;
    PMOS_RESOURCE           presIndirectObjectBuffer;
    uint32_t                dwIndirectObjectBufferSize;
    PMOS_RESOURCE           presInstructionBuffer;
    uint32_t                dwInstructionBufferSize;
    uint32_t                mocs4InstructionCache;
    uint32_t                mocs4GeneralState;
    uint32_t                mocs4DynamicState;
    uint32_t                mocs4SurfaceState;
    uint32_t                mocs4IndirectObjectBuffer;
    uint32_t                mocs4StatelessDataport;
    uint32_t                l1CacheConfig;
} MHW_STATE_BASE_ADDR_PARAMS, *PMHW_STATE_BASE_ADDR_PARAMS;

typedef struct _MHW_VFE_SCOREBOARD_DELTA
{
    uint8_t    x : 4;
    uint8_t    y : 4;
} MHW_VFE_SCOREBOARD_DELTA, *PMHW_VFE_SCOREBOARD_DELTA;

typedef struct _MHW_VFE_SCOREBOARD
{
    struct
    {
        uint32_t   ScoreboardMask      : 8;
        uint32_t   ScoreboardColor     : 4;
        uint32_t                       : 18;
        uint32_t   ScoreboardType      : 1;
        uint32_t   ScoreboardEnable    : 1;
    };

    union
    {
        MHW_VFE_SCOREBOARD_DELTA ScoreboardDelta[MHW_MAX_DEPENDENCY_COUNT];
        struct
        {
            uint32_t   Value[2];
        };
    };
} MHW_VFE_SCOREBOARD, *PMHW_VFE_SCOREBOARD;

struct MHW_VFE_PARAMS
{
    uint32_t                        dwDebugCounterControl = 0;      // Debug Counter Control
    uint32_t                        dwMaximumNumberofThreads = 0;
    uint32_t                        dwNumberofURBEntries = 0;
    uint32_t                        dwCURBEAllocationSize = 0;
    uint32_t                        dwURBEntryAllocationSize = 0;
    uint32_t                        dwPerThreadScratchSpace = 0;
    uint32_t                        dwScratchSpaceBasePointer = 0;
    MHW_VFE_SLICE_DISABLE           eVfeSliceDisable = MHW_VFE_SLICE_ALL;
    MHW_VFE_SCOREBOARD              Scoreboard = {};
    PMHW_KERNEL_STATE               pKernelState = nullptr;
    virtual ~MHW_VFE_PARAMS() {}
};
typedef MHW_VFE_PARAMS *PMHW_VFE_PARAMS;

typedef struct _MHW_CURBE_LOAD_PARAMS
{
    PMHW_KERNEL_STATE   pKernelState;
    bool                bOldInterface;
    uint32_t            dwCURBETotalDataLength;
    uint32_t            dwCURBEDataStartAddress;
} MHW_CURBE_LOAD_PARAMS, *PMHW_CURBE_LOAD_PARAMS;

typedef struct _MHW_ID_LOAD_PARAMS
{
    PMHW_KERNEL_STATE   pKernelState;
    uint32_t            dwNumKernelsLoaded;
    uint32_t            dwIdIdx;
    uint32_t            dwInterfaceDescriptorStartOffset;
    uint32_t            dwInterfaceDescriptorLength;
} MHW_ID_LOAD_PARAMS, *PMHW_ID_LOAD_PARAMS;

typedef struct _MHW_SIP_STATE_PARAMS
{
    bool                bSipKernel;
    uint32_t            dwSipBase;
} MHW_SIP_STATE_PARAMS, *PMHW_SIP_STATE_PARAMS;

typedef struct _MHW_WALKER_XY
{
    union
    {
        struct
        {
            uint32_t   x   : 16;
            uint32_t   y   : 16;
        };
        uint32_t       value;
    };
} MHW_WALKER_XY, *PMHW_WALKER_XY;

typedef struct _MHW_PALETTE_PARAMS
{
    int32_t                 iPaletteID;        //!< Palette ID
    int32_t                 iNumEntries;       //!< Palette entries in use
    void*                   pPaletteData;      //!< Palette data
} MHW_PALETTE_PARAMS, *PMHW_PALETTE_PARAMS;

typedef struct _MHW_CHROMAKEY_PARAMS
{
    uint32_t                dwIndex;            //!< Chroma Key Index
    uint32_t                dwLow;              //!< Chroma Key Low
    uint32_t                dwHigh;             //!< Chroma Key High
} MHW_CHROMAKEY_PARAMS, *PMHW_CHROMAKEY_PARAMS;

// IMPORTANT - changes in this structure must
// be ported to CM_HAL_WALKER_PARAMS in cm_common.h
typedef struct _MHW_WALKER_PARAMS
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
    uint8_t*                pInlineData;
    uint32_t                dwLocalLoopExecCount;
    uint32_t                dwGlobalLoopExecCount;

    MHW_WALKER_MODE         WalkerMode;
    MHW_WALKER_XY           BlockResolution;
    MHW_WALKER_XY           LocalStart;
    MHW_WALKER_XY           LocalEnd;
    MHW_WALKER_XY           LocalOutLoopStride;
    MHW_WALKER_XY           LocalInnerLoopUnit;
    MHW_WALKER_XY           GlobalResolution;
    MHW_WALKER_XY           GlobalStart;
    MHW_WALKER_XY           GlobalOutlerLoopStride;
    MHW_WALKER_XY           GlobalInnerLoopUnit;

    bool                    bAddMediaFlush;
    bool                    bRequestSingleSlice;

    uint32_t                IndirectDataLength;
    uint32_t                IndirectDataStartAddress;
} MHW_WALKER_PARAMS, *PMHW_WALKER_PARAMS;

typedef struct _MHW_GPGPU_WALKER_PARAMS
{
    uint32_t                   InterfaceDescriptorOffset   : 5;
    uint32_t                   GpGpuEnable                 : 1;
    uint32_t                                               : 26;
    uint32_t                   ThreadWidth;
    uint32_t                   ThreadHeight;
    uint32_t                   ThreadDepth;
    uint32_t                   GroupWidth;
    uint32_t                   GroupHeight;
    uint32_t                   GroupDepth;
    uint32_t                   GroupStartingX;
    uint32_t                   GroupStartingY;
    uint32_t                   GroupStartingZ;
    uint32_t                   SLMSize;

    uint32_t                   IndirectDataLength;
    uint32_t                   IndirectDataStartAddress;
    uint32_t                   BindingTableID;
} MHW_GPGPU_WALKER_PARAMS, *PMHW_GPGPU_WALKER_PARAMS;

typedef struct _MHW_MEDIA_OBJECT_PARAMS
{
    uint32_t                            dwInterfaceDescriptorOffset;
    uint32_t                            dwHalfSliceDestinationSelect;
    uint32_t                            dwSliceDestinationSelect;
    uint32_t                            dwIndirectLoadLength;
    uint32_t                            dwIndirectDataStartAddress;
    void*                               pInlineData;
    uint32_t                            dwInlineDataSize;
    bool                                bForceDestination;
    MHW_VFE_SCOREBOARD                  VfeScoreboard;
} MHW_MEDIA_OBJECT_PARAMS, *PMHW_MEDIA_OBJECT_PARAMS;

class MhwRenderInterface
{
public:
    PMHW_STATE_HEAP_INTERFACE m_stateHeapInterface = nullptr;

    virtual ~MhwRenderInterface()
    {
        if (m_stateHeapInterface)
        {
            m_stateHeapInterface->pfnDestroy(m_stateHeapInterface);
        }
    }

    //!
    //! \brief    Allocates the MHW render interface internal parameters
    //! \details  Internal MHW function to allocate all parameters needed for the
    //!           render interface including the state heap interface
    //! \param    MHW_STATE_HEAP_SETTINGS stateHeapSettings
    //!           [in] Setting used to initialize the state heap interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateHeaps(
        MHW_STATE_HEAP_SETTINGS         stateHeapSettings);

    //!
    //! \brief    Adds PIPELINE_SELECT to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which commands are added
    //! \param    gpGpuPipe
    //!           [in] false: MEDIA pipe; true:  GPGPU pipe
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPipelineSelectCmd (
        PMOS_COMMAND_BUFFER             cmdBuffer,
        bool                            gpGpuPipe) = 0;

    //!
    //! \brief    Adds STATE_BASE_ADDRESS to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddStateBaseAddrCmd(
        PMOS_COMMAND_BUFFER                 cmdBuffer,
        PMHW_STATE_BASE_ADDR_PARAMS         params) = 0;

    //!
    //! \brief    Adds MEDIA_VFE_STATE to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaVfeCmd (
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VFE_PARAMS                 params) = 0;

    //!
    //! \brief    Adds CFE_STATE to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddCfeStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_VFE_PARAMS                 params)
    {
        MOS_UNUSED(cmdBuffer);
        MOS_UNUSED(params);

        // CFE_STATE will replace the MEDIA_VFE_STATE on some platform; Just keep the
        // platform which really uses CFE to implement it on inheriting class .
        MHW_ASSERTMESSAGE("Don't support it on this platform");
        return MOS_STATUS_SUCCESS;
    }

    //!
    //! \brief    Adds MEDIA_CURBE_LOAD to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaCurbeLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_CURBE_LOAD_PARAMS          params) = 0;

    //!
    //! \brief    Adds MEDIA_INTERFACE_DESCRIPTOR_LOAD to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaIDLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_ID_LOAD_PARAMS             params) = 0;

    //!
    //! \brief    Adds MEDIA_OBJECT to the buffer provided
    //! \details  MEDIA_OBJCET is added to either the command buffer or
    //!           batch buffer (whichever is valid)
    //! \param    cmdBuffer
    //!           [in] If valid, command buffer to which HW command is added
    //! \param    batchBuffer
    //!           [in] If valid, Batch buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaObject(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_BATCH_BUFFER               batchBuffer,
        PMHW_MEDIA_OBJECT_PARAMS        params) = 0;

    //!
    //! \brief    Adds MEDIA_OBJECT_WALKER to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddMediaObjectWalkerCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_WALKER_PARAMS              params) = 0;

    //!
    //! \brief    Adds GPGPU_WALKER to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddGpGpuWalkerStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_GPGPU_WALKER_PARAMS        params) = 0;

    //!
    //! \brief    Adds 3DSTATE_CHROMA_KEY to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddChromaKeyCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_CHROMAKEY_PARAMS           params) = 0;

    //!
    //! \brief    Adds 3DSTATE_SAMPLER_PALETTE_LOADX to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddPaletteLoadCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_PALETTE_PARAMS             params) = 0;

    //!
    //! \brief    Adds STATE_SIP to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    params
    //!           [in] Params structure used to populate the HW command
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddSipStateCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_SIP_STATE_PARAMS           params) = 0;

    //!
    //! \brief    Adds GPGPU_CSR_BASE_ADDRESS to the command buffer
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW command is added
    //! \param    csrResource
    //!           [in] Resource to be used for GPGPU CSR
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS AddGpgpuCsrBaseAddrCmd(
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMOS_RESOURCE                   csrResource) = 0;

    //!
    //! \brief    get the size of hw command
    //! \details  Internal function to get the size of MEDIA_OBJECT_CMD
    //! \return   commandSize
    //!           The command size
    //!
    virtual uint32_t GetMediaObjectCmdSize() = 0;

    //!
    //! \brief    Enables L3 cacheing flag and sets related registers/values
    //! \param    cacheSettings
    //!           [in] L3 Cache Configurations, if a null pointer is passed 
    //!           in, it will use default settings.
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS EnableL3Caching(
        PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS    cacheSettings) = 0;

    //!
    //! \brief    Setup L3 cache configuration for kernel workload
    //! \details  Enable L3 cacheing in kernel workload by configuring the
    //!           appropriate MMIO registers.
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW commands is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS SetL3Cache(
        PMOS_COMMAND_BUFFER             cmdBuffer) = 0;

    //!
    //! \brief    Enables preemption for media workloads on render engine
    //! \details  Sets the MMIO register for preemption so that HW can preempt
    //!           the submitted workload if required
    //! \param    cmdBuffer
    //!           [in] Command buffer to which HW commands is added
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS EnablePreemption(
        PMOS_COMMAND_BUFFER             cmdBuffer);

    //!
    //! \brief    Accessor for m_l3CacheConfig
    //! \return   L3 cache configuration information
    //!
    virtual MHW_RENDER_ENGINE_L3_CACHE_CONFIG* GetL3CacheConfig() = 0;

    //!
    //! \brief    Accessor for m_hwCaps, temporarily returns pointer to m_hwCaps until clients move to using not a pointer
    //! \return   Pointer to HW capabilities
    //!
    MHW_RENDER_ENGINE_CAPS *GetHwCaps() { return &m_hwCaps; }

    //!
    //! \brief    Accessor for m_preemptionEnabled
    //! \return   true if preemption is enabled, false otherwise
    //!
    bool IsPreemptionEnabled() { return m_preemptionEnabled; }

    //!
    //! \brief    Setter for os interface, used in MFE scenario
    //! \return   void
    //!
    void SetOsInterface(PMOS_INTERFACE osInterface) { m_osInterface = osInterface;}

    //!
    //! \brief    Get mmio registers address
    //! \details  Get mmio registers address
    //! \return   [out] PMHW_MI_MMIOREGISTERS*
    //!           mmio registers got.
    //!
    virtual PMHW_MI_MMIOREGISTERS GetMmioRegisters() = 0;

protected:
    //!
    //! \brief    Initializes the Render interface
    //! \details  Internal MHW function to initialize all function pointers and some parameters
    //!           Assumes that the caller has checked pointer validity and whether or not an
    //!           addressing method has been selected in the OS interface (bUsesGfxAddress or
    //!           bUsesPatchList).
    //! \param    [in] miInterface
    //!           MI interface, must be valid
    //! \param    [in] osInterface
    //!           OS interface, must be valid
    //! \param    [in] gtSystemInfo
    //!           System information, must be valid
    //! \param    [in] newStateHeapManagerRequested
    //!           A new state heap manager was implemented for MDF, will be adapted for codec & VP,
    //!           migrated to C++, rolled into the existing state heap interface and removed.
    //!           Ultimately this parameter will no longer be necessary as the state heap interface
    //!           will be unified.
    //!
    MhwRenderInterface(
        MhwMiInterface          *miInterface,
        PMOS_INTERFACE          osInterface,
        MEDIA_SYSTEM_INFO       *gtSystemInfo,
        uint8_t                 newStateHeapManagerRequested)
    {
        MHW_FUNCTION_ENTER;

        if (miInterface == nullptr ||
            osInterface == nullptr ||
            gtSystemInfo == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid input pointers provided");
            return;
        }

        if (!osInterface->bUsesGfxAddress && !osInterface->bUsesPatchList)
        {
            MHW_ASSERTMESSAGE("No valid addressing mode indicated");
            return;
        }

        m_osInterface = osInterface;
        m_miInterface = miInterface;
        m_stateHeapInterface = nullptr;

        memset(&m_hwCaps, 0, sizeof(m_hwCaps));

        if (m_osInterface->bUsesGfxAddress)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
        }
        else // if (m_osInterface->bUsesPatchList)
        {
            AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
        }

        InitPlatformCaps(gtSystemInfo);

        InitPreemption();

        if (Mhw_StateHeapInterface_InitInterface(
            &m_stateHeapInterface,
            m_osInterface,
            newStateHeapManagerRequested) != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("State heap initialization failed!");
            return;
        }
    }

    PMOS_INTERFACE      m_osInterface = nullptr;
    MEDIA_FEATURE_TABLE *m_skuTable = nullptr;
    MhwMiInterface      *m_miInterface = nullptr;

    MHW_RENDER_ENGINE_L3_CACHE_CONFIG   m_l3CacheConfig;
    MHW_RENDER_ENGINE_CAPS              m_hwCaps;

    bool        m_preemptionEnabled = false;
    uint32_t    m_preemptionCntlRegisterOffset = 0;
    uint32_t    m_preemptionCntlRegisterValue = 0;

    uint32_t    m_l3CacheCntlRegisterOffset = M_L3_CACHE_CNTL_REG_OFFSET;
    uint32_t    m_l3CacheCntlRegisterValueDefault = M_L3_CACHE_CNTL_REG_VALUE_DEFAULT;

    //!
    //! \brief    Adds a resource to the command buffer or indirect state (SSH)
    //! \details  Internal MHW function to add either a graphics address of a resource or
    //!           add the resource to the patch list for the requested buffer or state
    //! \param    [in] osInterface
    //!           OS interface
    //! \param    [in] cmdBuffer
    //!           If adding a resource to the command buffer, the buffer to which the resource
    //!           is added
    //! \param    [in] params
    //!           Parameters necessary to add the graphics address
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS(*AddResourceToCmd) (
        PMOS_INTERFACE                  osInterface,
        PMOS_COMMAND_BUFFER             cmdBuffer,
        PMHW_RESOURCE_PARAMS            params);

    //!
    //! \brief    Initializes platform related capabilities for the render engine
    //! \details  Assumes the caller checked the pointers for validity.
    //! \param    gtSystemInfo
    //!           [in] Information concerning the GPU
    //! \return   void
    //!
    void InitPlatformCaps(
        MEDIA_SYSTEM_INFO         *gtSystemInfo);

    //!
    //! \brief    Initializes preemption related registers/values
    //! \details  Initializes the MMIO register for preemption so that HW can preempt
    //!           the submitted workload if required.
    //! \return   void
    //!           If invalid SKU\WA tables detected, does not do anything
    //!
    void InitPreemption();
};

#endif // __MHW_RENDER_H__
