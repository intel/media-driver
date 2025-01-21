/*
* Copyright (c) 2014-2021, Intel Corporation
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
#include "mhw_state_heap.h"
#include "mhw_render_itf.h"

#define MHW_RENDER_ENGINE_SSH_SURFACES_PER_BT_MAX           256
#define MHW_RENDER_ENGINE_SAMPLERS_MAX                      16
#define MHW_RENDER_ENGINE_SAMPLERS_AVS_MAX                  8
#define MHW_RENDER_ENGINE_MEDIA_PALOAD_SIZE_MAX             512
#define MHW_RENDER_ENGINE_URB_SIZE_MAX                      2048
#define MHW_RENDER_ENGINE_URB_ENTRIES_MAX                   128
#define MHW_RENDER_ENGINE_INTERFACE_DESCRIPTOR_ENTRIES_MAX  64
#define MHW_RENDER_ENGINE_EU_INDEX_MAX                      12
#define MHW_RENDER_ENGINE_SIZE_REGISTERS_PER_THREAD         0x1800
#define MHW_RENDER_ENGINE_NUMBER_OF_THREAD_UNIT             32

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
    MHW_RENDER_ENGINE_KERNEL_POINTER_SHIFT = 6,
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

typedef enum _MHW_EMIT_LOCAL_MODE
{
    MHW_EMIT_LOCAL_NONE = 0,
    MHW_EMIT_LOCAL_X    = 1,
    MHW_EMIT_LOCAL_XY   = 3,
    MHW_EMIT_LOCAL_XYZ  = 7
} MHW_EMIT_LOCAL_MODE;

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
    bool                    addressDis;
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
    uint32_t                   ForcePreferredSLMZero;

    bool                       isEmitInlineParameter;
    uint32_t                   inlineDataLength;
    uint8_t*                   inlineData;

    bool                       isGenerateLocalID;
    MHW_EMIT_LOCAL_MODE        emitLocal;

    bool                       hasBarrier;
    PMHW_INLINE_DATA_PARAMS    inlineDataParamBase;
    uint32_t                   inlineDataParamSize;

    uint32_t                   simdSize;

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

#endif // __MHW_RENDER_H__