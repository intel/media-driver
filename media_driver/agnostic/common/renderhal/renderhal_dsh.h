/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file      renderhal_dsh.h 
//! \brief         This module defines render engine state heap management functions based     on dynamic state heap (DSH) infrastructure, rather than static state heap. 
//!
#ifndef __RENDERHAL_DSH_H__
#define __RENDERHAL_DSH_H__

#include "mhw_state_heap.h"
#include "heap.h"

// Absolute max number of interface descriptors
#define RENDERHAL_DSH_MAX_MEDIA_IDs 16

// Media dynamic state pool increment
#define RENDERHAL_DSH_DYN_STATE_INC 16

// Kernel allocation pool increment
#define RENDERHAL_DSH_KRN_ALLOC_INC 16

typedef struct _RENDERHAL_REGION
{
    int32_t                 iCount;             // Total Object count
    int32_t                 iCurrent;           // Current Object count
    uint32_t                dwOffset;           // Region offset (from Media State Base)
    uint32_t                dwSize;             // Region size
} RENDERHAL_REGION;

// Forward declarations
typedef struct _RENDERHAL_KRN_ALLOCATION *PRENDERHAL_KRN_ALLOCATION;
typedef struct _RENDERHAL_INTERFACE      *PRENDERHAL_INTERFACE;

typedef struct _RENDERHAL_DYNAMIC_STATE *PRENDERHAL_DYNAMIC_STATE;

typedef struct _RENDERHAL_DYNAMIC_STATE
{
    // Memory block associated with the media state
    MemoryBlock                 memoryBlock;                                  // block associated with the Media State

    // Dynamic media state regions
    uint32_t                    dwSizeSamplers;                                 // Total samplers size per MediaID
    RENDERHAL_REGION            Curbe;                                          // CURBE data
    RENDERHAL_REGION            MediaID;                                        // Media Interface Descriptors
    RENDERHAL_REGION            Sampler3D;                                      // 3D Sampler states (also sampler base)
    RENDERHAL_REGION            SamplerAVS;                                     // AVS Sampler states
    RENDERHAL_REGION            SamplerConv;                                    // Conv Sampler states
    RENDERHAL_REGION            SamplerMisc;                                    // Misc (VA) Sampler states
    RENDERHAL_REGION            SamplerInd;                                     // Indirect Sampler states
    RENDERHAL_REGION            Table8x8;                                       // AVS Sampler 8x8 tables
    RENDERHAL_REGION            Performance;                                    // Performance collection

    // State allocations
    int32_t                     iMaxScratchSpacePerThread;                      // Max per thread scratch space reported by JITTER
    uint32_t                    dwScratchSpace;                                 // Size of scratch space needed
    uint32_t                    scratchSpaceOffset;                             // Offset to scratch space within the memory block
    int32_t                     iCurrentBindingTable;                           // Current binding table
    int32_t                     iCurrentSurfaceState;                           // Current surface state
    PRENDERHAL_KRN_ALLOCATION   pKrnAllocations[RENDERHAL_DSH_MAX_MEDIA_IDs];   // Media Kernel Allocations (1:1 mapping with Media IDs)
} RENDERHAL_DYNAMIC_STATE, *PRENDERHAL_DYNAMIC_STATE;

//---------------------------
// Dynamic State Heap Objects
//---------------------------
typedef struct _RENDERHAL_DYN_HEAP_SETTINGS
{
    // Dynamic State Heap Settings
    uint32_t                dwDshInitialSize;
    uint32_t                dwDshSizeIncrement;
    uint32_t                dwDshMaximumSize;

    // Instruction Heap Settings
    uint32_t                dwIshInitialSize;
    uint32_t                dwIshSizeIncrement;
    uint32_t                dwIshMaximumSize;

    // Media State settings
    int32_t                 iMinMediaStates;
    int32_t                 iMaxMediaStates;

    // Kernel State settings
    int32_t                 iMinKernels;
    int32_t                 iMaxKernels;
} RENDERHAL_DYN_HEAP_SETTINGS, *PRENDERHAL_DYN_HEAP_SETTINGS;

#endif // __RENDERHAL_DSH_H__
