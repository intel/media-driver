/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     mhw_render_cmdpar.h
//! \brief    MHW command parameters
//! \details
//!

#ifndef __MHW_RENDER_CMDPAR_H__
#define __MHW_RENDER_CMDPAR_H__

#include "mhw_cmdpar.h"
#include "mhw_state_heap.h"

static constexpr uint32_t MHW_RENDER_ENGINE_SSH_SURFACES_PER_BT_MAX          = 256;
static constexpr uint32_t MHW_RENDER_ENGINE_SAMPLERS_MAX                     = 16;
static constexpr uint32_t MHW_RENDER_ENGINE_SAMPLERS_AVS_MAX                 = 8;
static constexpr uint32_t MHW_RENDER_ENGINE_MEDIA_PALOAD_SIZE_MAX            = 512;
static constexpr uint32_t MHW_RENDER_ENGINE_URB_SIZE_MAX                     = 2048;
static constexpr uint32_t MHW_RENDER_ENGINE_URB_ENTRIES_MAX                  = 128;
static constexpr uint32_t MHW_RENDER_ENGINE_INTERFACE_DESCRIPTOR_ENTRIES_MAX = 64;
static constexpr uint32_t MHW_RENDER_ENGINE_EU_INDEX_MAX                     = 12;
static constexpr uint32_t MHW_RENDER_ENGINE_SIZE_REGISTERS_PER_THREAD        = 0x1800;
static constexpr uint32_t MHW_MAX_DEPENDENCY_COUNT                           = 8;

namespace mhw
{
namespace render
{
struct MHW_VFE_SCOREBOARD_DELTA
{
    uint8_t    x = 0;
    uint8_t    y = 0;
};

struct MHW_VFE_SCOREBOARD
{
    uint8_t   ScoreboardMask  = 0;
    uint8_t   ScoreboardColor = 0;
    bool      ScoreboardType  = false;
    bool     ScoreboardEnable = false;
    MHW_VFE_SCOREBOARD_DELTA ScoreboardDelta[MHW_MAX_DEPENDENCY_COUNT] = {};
};

struct MHW_HEAPS_RESOURCE
{
    PMOS_RESOURCE          presInstructionBuffer = nullptr;
    PMHW_INLINE_DATA_PARAMS inlineDataParamsBase   = nullptr;
    uint32_t                inlineDataParamSize   = 0;
};

enum MHW_VFE_SLICE_DISABLE
{
    MHW_VFE_SLICE_ALL = 0,
    MHW_VFE_SLICE0_SUBSLICE_ALL,
    MHW_VFE_SLICE0_SUBSLICE0
};

struct MHW_WALKER_XY
{
    uint16_t   x = 0;
    uint16_t   y = 0;
};

enum MHW_WALKER_MODE
{
    MHW_WALKER_MODE_NOT_SET  = -1,
    MHW_WALKER_MODE_DISABLED = 0,
    MHW_WALKER_MODE_SINGLE   = 1,    // dual = 0, repel = 1
    MHW_WALKER_MODE_DUAL     = 2,    // dual = 1, repel = 0)
    MHW_WALKER_MODE_TRI      = 3,    // applies in BDW GT2 which has 1 slice and 3 sampler/VME per slice
    MHW_WALKER_MODE_QUAD     = 4,    // applies in HSW GT3 which has 2 slices and 2 sampler/VME per slice
    MHW_WALKER_MODE_HEX      = 6,    // applies in BDW GT2 which has 2 slices and 3 sampler/VME per slice
    MHW_WALKER_MODE_OCT      = 8     // may apply in future Gen media architectures
};

enum MHW_EMIT_LOCAL_MODE
{
    MHW_EMIT_LOCAL_NONE = 0,
    MHW_EMIT_LOCAL_X    = 1,
    MHW_EMIT_LOCAL_XY   = 3,
    MHW_EMIT_LOCAL_XYZ  = 7
};

struct MHW_RENDER_ENGINE_CAPS
{
    uint32_t               dwMaxUnormSamplers              = 0;                    // Max UNORM Sampler States supported
    uint32_t               dwMaxAVSSamplers                = 0;                    // Max AVS Sampler States supported
    uint32_t               dwMaxBTIndex                    = 0;                    // Max Binding Table index per Binding Table
    uint32_t               dwMaxThreads                    = 0;                    // Max Threads supported
    uint32_t               dwMaxMediaPayloadSize           = 0;                    // Max Media payload size
    uint32_t               dwMaxURBSize                    = 0;                    // Max URB Size
    uint32_t               dwMaxURBEntries                 = 0;                    // Max URB Entries
    uint32_t               dwMaxURBEntryAllocationSize     = 0;                    // Max URB Entry Allocation Size
    uint32_t               dwMaxCURBEAllocationSize        = 0;
    uint32_t               dwMaxInterfaceDescriptorEntries = 0;                    // Max Interface Descriptor Entries
    uint32_t               dwMaxSubslice                   = 0;                    // Max number of subslice
    uint32_t               dwMaxEUIndex                    = 0;                    // Max EU index (sometimes != number of EU)
    uint32_t               dwNumThreadsPerEU               = 0;                    // Num threads per EU
    uint32_t               dwSizeRegistersPerThread        = 0;                    // Size of all registers per thread (for ASM Debug)
};

struct MHW_RENDER_ENGINE_L3_CACHE_SETTINGS
{
    uint32_t   dwCntlReg      = 0;
    uint32_t   dwCntlReg2     = 0;
    uint32_t   dwCntlReg3     = 0;
    uint32_t   dwSqcReg1      = 0;
    uint32_t   dwSqcReg4      = 0;
    uint32_t   dwLra1Reg      = 0;
    uint32_t   dwTcCntlReg    = 0;
    uint32_t   dwAllocReg     = 0;
    bool       bUpdateDefault = 0;
};

struct MHW_RENDER_ENGINE_L3_CACHE_CONFIG
{
    uint32_t   dwL3CacheCntlReg_Register       = 0;
    uint32_t   dwL3CacheCntlReg_Setting        = 0;
    uint32_t   dwL3CacheCntlReg2_Register      = 0;
    uint32_t   dwL3CacheCntlReg2_Setting       = 0;
    uint32_t   dwL3CacheCntlReg3_Register      = 0;
    uint32_t   dwL3CacheCntlReg3_Setting       = 0;
    uint32_t   dwL3CacheSqcReg1_Register       = 0;
    uint32_t   dwL3CacheSqcReg1_Setting        = 0;
    uint32_t   dwL3CacheSqcReg4_Register       = 0;
    uint32_t   dwL3CacheSqcReg4_Setting        = 0;
    uint32_t   dwL3LRA1Reg_Register            = 0;
    uint32_t   dwL3LRA1Reg_Setting             = 0;
    bool       bL3CachingEnabled               = false;
    bool       bL3LRA1Reset                    = false;
    uint32_t   dwRcsL3CacheTcCntlReg_Register  = 0;
    uint32_t   dwL3CacheTcCntlReg_Setting      = 0;
    uint32_t   dwRcsL3CacheAllocReg_Register   = 0;
    uint32_t   dwL3CacheAllocReg_Setting       = 0;
    uint32_t   dwCcs0L3CacheTcCntlReg_Register = 0;
    uint32_t   dwCcs0L3CacheAllocReg_Register  = 0;

};

struct MHW_VFE_PARAMS
{
    uint32_t                        dwDebugCounterControl            = 0;      // Debug Counter Control
    uint32_t                        dwMaximumNumberofThreads         = 0;
    uint32_t                        dwNumberofURBEntries             = 0;
    uint32_t                        dwCURBEAllocationSize            = 0;
    uint32_t                        dwURBEntryAllocationSize         = 0;
    uint32_t                        dwPerThreadScratchSpace          = 0;
    uint32_t                        dwScratchSpaceBasePointer        = 0;
    MHW_VFE_SLICE_DISABLE           eVfeSliceDisable                 = MHW_VFE_SLICE_ALL;
    MHW_VFE_SCOREBOARD              Scoreboard                       = {};
    PMHW_KERNEL_STATE               pKernelState                     = nullptr;
    bool                            bFusedEuDispatch                 = 0;
    uint32_t                        numOfWalkers                     = 0;
    bool                            enableSingleSliceDispatchCcsMode = 0;
    // Surface state offset of scratch space buffer.
    uint32_t                        scratchStateOffset               = 0;
};

struct _MHW_PAR_T(MEDIA_OBJECT)
{
    uint32_t                            dwInterfaceDescriptorOffset  = 0;
    uint32_t                            dwHalfSliceDestinationSelect = 0;
    uint32_t                            dwSliceDestinationSelect     = 0;
    uint32_t                            dwIndirectLoadLength         = 0;
    uint32_t                            dwIndirectDataStartAddress   = 0;
    void*                               pInlineData                  = nullptr;
    uint32_t                            dwInlineDataSize             = 0;
    bool                                bForceDestination            = false;
    MHW_VFE_SCOREBOARD                  VfeScoreboard                = {};
};

struct _MHW_PAR_T(PIPE_MODE_SELECT)
{
    uint32_t           dwInterfaceDescriptorOffset  = 0;
    uint32_t           dwHalfSliceDestinationSelect = 0;
    uint32_t           dwSliceDestinationSelect     = 0;
    uint32_t           dwIndirectLoadLength         = 0;
    uint32_t           dwIndirectDataStartAddress   = 0;
    void*              pInlineData                  = nullptr;
    uint32_t           dwInlineDataSize             = 0;
    bool               bForceDestination            = false;
    MHW_VFE_SCOREBOARD VfeScoreboard                = {};
};

struct _MHW_PAR_T(PIPELINE_SELECT)
{
    uint32_t          pipelineSelection = 0;
    bool              gpGpuPipe         = false;
};

struct _MHW_PAR_T(STATE_BASE_ADDRESS)
{
    PMOS_RESOURCE           presGeneralState           = nullptr;
    uint32_t                dwGeneralStateSize         = 0;
    PMOS_RESOURCE           presDynamicState           = nullptr;
    uint32_t                dwDynamicStateSize         = 0;
    bool                    bDynamicStateRenderTarget  = false;
    PMOS_RESOURCE           presIndirectObjectBuffer   = nullptr;
    uint32_t                dwIndirectObjectBufferSize = 0;
    PMOS_RESOURCE           presInstructionBuffer      = nullptr;
    uint32_t                dwInstructionBufferSize    = 0;
    uint32_t                mocs4InstructionCache      = 0;
    uint32_t                mocs4GeneralState          = 0;
    uint32_t                mocs4DynamicState          = 0;
    uint32_t                mocs4SurfaceState          = 0;
    uint32_t                mocs4IndirectObjectBuffer  = 0;
    uint32_t                mocs4StatelessDataport     = 0;
    uint32_t                l1CacheConfig              = 0;
    bool                    addressDis                 = false;
};

struct _MHW_PAR_T(MEDIA_VFE_STATE)
{
    uint32_t                        dwDebugCounterControl     = 0;      // Debug Counter Control
    uint32_t                        dwMaximumNumberofThreads  = 0;
    uint32_t                        dwNumberofURBEntries      = 0;
    uint32_t                        dwCURBEAllocationSize     = 0;
    uint32_t                        dwURBEntryAllocationSize  = 0;
    uint32_t                        dwPerThreadScratchSpace   = 0;
    uint32_t                        dwScratchSpaceBasePointer = 0;
    MHW_VFE_SLICE_DISABLE           eVfeSliceDisable          = MHW_VFE_SLICE_ALL;
    MHW_VFE_SCOREBOARD              Scoreboard                = {};
    PMHW_KERNEL_STATE               pKernelState              = nullptr;
    MHW_RENDER_ENGINE_CAPS*         pHwCaps                   = nullptr;
};

struct _MHW_PAR_T(MEDIA_CURBE_LOAD)
{
    PMHW_KERNEL_STATE   pKernelState            = nullptr;
    bool                bOldInterface           = false;
    uint32_t            dwCURBETotalDataLength  = 0;
    uint32_t            dwCURBEDataStartAddress = 0;
};

struct _MHW_PAR_T(MEDIA_INTERFACE_DESCRIPTOR_LOAD)
{
    PMHW_KERNEL_STATE   pKernelState                     = nullptr;
    uint32_t            dwNumKernelsLoaded               = 0;
    uint32_t            dwIdIdx                          = 0;
    uint32_t            dwInterfaceDescriptorStartOffset = 0;
    uint32_t            dwInterfaceDescriptorLength      = 0;
};

struct _MHW_PAR_T(MEDIA_OBJECT_WALKER)
{
    uint16_t                InterfaceDescriptorOffset = 0;
    bool                    CmWalkerEnable            = false;
    uint16_t                ColorCountMinusOne        = 0;
    uint16_t                UseScoreboard             = 0;
    uint16_t                ScoreboardMask            = 0;
    uint8_t                 MidLoopUnitX              = 0;
    uint8_t                 MidLoopUnitY              = 0;
    uint8_t                 MiddleLoopExtraSteps      = 0;
    uint32_t                GroupIdLoopSelect         = 0;
    uint32_t                InlineDataLength          = 0;
    uint8_t*                pInlineData               = nullptr ;
    uint32_t                dwLocalLoopExecCount      = 0;
    uint32_t                dwGlobalLoopExecCount     = 0;
    MHW_WALKER_MODE         WalkerMode                = {};
    MHW_WALKER_XY           BlockResolution           = {};
    MHW_WALKER_XY           LocalStart                = {};
    MHW_WALKER_XY           LocalEnd                  = {};
    MHW_WALKER_XY           LocalOutLoopStride        = {};
    MHW_WALKER_XY           LocalInnerLoopUnit        = {};
    MHW_WALKER_XY           GlobalResolution          = {};
    MHW_WALKER_XY           GlobalStart               = {};
    MHW_WALKER_XY           GlobalOutlerLoopStride    = {};
    MHW_WALKER_XY           GlobalInnerLoopUnit       = {};
    bool                    bAddMediaFlush            = false;
    bool                    bRequestSingleSlice       = false;
    uint32_t                IndirectDataLength        = 0;
    uint32_t                IndirectDataStartAddress  = 0;
};

struct _MHW_PAR_T(GPGPU_WALKER)
{
    uint8_t                 InterfaceDescriptorOffset = 0;
    bool                    GpGpuEnable               = false;
    uint32_t                ThreadWidth               = 0;
    uint32_t                ThreadHeight              = 0;
    uint32_t                ThreadDepth               = 0;
    uint32_t                GroupWidth                = 0;
    uint32_t                GroupHeight               = 0;
    uint32_t                GroupDepth                = 0;
    uint32_t                GroupStartingX            = 0;
    uint32_t                GroupStartingY            = 0;
    uint32_t                GroupStartingZ            = 0;
    uint32_t                SLMSize                   = 0;
    uint32_t                IndirectDataLength        = 0;
    uint32_t                IndirectDataStartAddress  = 0;
    uint32_t                BindingTableID            = 0;
};

struct _MHW_PAR_T(_3DSTATE_CHROMA_KEY)
{
    uint32_t                dwIndex = 0;            //!< Chroma Key Index
    uint32_t                dwLow   = 0;            //!< Chroma Key Low
    uint32_t                dwHigh  = 0;            //!< Chroma Key High
};

struct _MHW_PAR_T(PALETTE_ENTRY)
{
    int32_t                 iPaletteID   = 0;       //!< Palette ID
    int32_t                 iNumEntries  = 0;       //!< Palette entries in use
    void*                   pPaletteData = nullptr; //!< Palette data
};

struct _MHW_PAR_T(STATE_SIP)
{
    bool                bSipKernel = false;
    uint32_t            dwSipBase  = 0;
};

struct _MHW_PAR_T(GPGPU_CSR_BASE_ADDRESS)
{
};

struct _MHW_PAR_T(_3DSTATE_BINDING_TABLE_POOL_ALLOC)
{
    uint32_t mocs4SurfaceState = 0;
};

struct _MHW_PAR_T(CFE_STATE)
{
    uint32_t                        dwDebugCounterControl      = 0;      // Debug Counter Control
    uint32_t                        dwMaximumNumberofThreads   = 0;
    uint32_t                        dwNumberofURBEntries       = 0;
    uint32_t                        dwCURBEAllocationSize      = 0;
    uint32_t                        dwURBEntryAllocationSize   = 0;
    uint32_t                        dwPerThreadScratchSpace    = 0;
    uint32_t                        dwScratchSpaceBasePointer  = 0;
    uint32_t                        ScratchSpaceBuffer         = 0;
    bool                            FusedEuDispatch            = 0;
    bool                            NumberOfWalkers            = 0;
    bool                            SingleSliceDispatchCcsMode = 0;
    MHW_VFE_SLICE_DISABLE           eVfeSliceDisable           = MHW_VFE_SLICE_ALL;
    MHW_VFE_SCOREBOARD              Scoreboard                 = {};
    PMHW_KERNEL_STATE               pKernelState               = nullptr;
};

struct _MHW_PAR_T(COMPUTE_WALKER)
{
    uint8_t                   InterfaceDescriptorOffset     = 0;
    bool                      GpGpuEnable                   = false;
    uint32_t                  ThreadWidth                   = 0;
    uint32_t                  ThreadHeight                  = 0;
    uint32_t                  ThreadDepth                   = 0;
    uint32_t                  GroupWidth                    = 0;
    uint32_t                  GroupHeight                   = 0;
    uint32_t                  GroupDepth                    = 0;
    uint32_t                  GroupStartingX                = 0;
    uint32_t                  GroupStartingY                = 0;
    uint32_t                  GroupStartingZ                = 0;
    uint32_t                  SLMSize                       = 0;
    uint32_t                  IndirectDataLength            = 0;
    uint32_t                  IndirectDataStartAddress      = 0;
    uint32_t                  BindingTableID                = 0;
    uint32_t                  dwMediaIdOffset               = 0;       //! Offset of the first Media Interface Descriptor (in DSH)
    uint32_t                  iMediaId                      = 0;       //! Media Interface Descriptor #
    uint32_t                  dwKernelOffset                = 0;       //! Kernel offset (in ISH)
    uint32_t                  dwSamplerOffset               = 0;       //! Sampler offset (in DSH)
    uint32_t                  dwSamplerCount                = 0;       //! Sample count
    uint32_t                  dwBindingTableOffset          = 0;       //! Binding table offset (in DSH)
    uint32_t                  iCurbeOffset                  = 0;       //! Curbe offset (in DSH)
    uint32_t                  iCurbeLength                  = 0;       //! Curbe lenght
    bool                      bBarrierEnable                = false;   //! Enable Barrier
    bool                      bGlobalBarrierEnable          = false;   //! Enable Global Barrier (SKL+)
    uint32_t                  dwNumberofThreadsInGPGPUGroup = 0;       //! Number of threads per group
    uint32_t                  dwSharedLocalMemorySize       = 0;       //! Size of SharedLocalMemory (SLM)
    int32_t                   forcePreferredSLMZero         = 0;       //! force preferredSLM value as 0
    int32_t                   iCrsThdConDataRdLn            = 0;
    PMHW_STATE_HEAP           pGeneralStateHeap             = 0;       //! General state heap in use
    MemoryBlock               *memoryBlock                  = nullptr; //! Memory block associated with the state heap
    MOS_RESOURCE              *postsyncResource             = nullptr;
    uint32_t                  resourceOffset                = 0;
    bool                      isEmitInlineParameter         = false;
    uint32_t                  inlineDataLength              = 0;
    uint8_t                   *inlineData                   = nullptr;
    bool                      isGenerateLocalId             = false;
    MHW_EMIT_LOCAL_MODE       emitLocal                     = MHW_EMIT_LOCAL_NONE;
    uint32_t                  preferredSlmAllocationSize    = 0;
    uint32_t                  simdSize                      = 0;
    _MHW_PAR_T(CFE_STATE)     cfeState                      = {};
    MHW_HEAPS_RESOURCE        heapsResource                 = {};

};

struct _MHW_PAR_T(STATE_COMPUTE_MODE)
{
    bool enableLargeGrf = false;
    uint32_t forceEuThreadSchedulingMode = 0;
};

}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_CMDPAR_H__
