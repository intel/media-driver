/*
* Copyright (c) 2018-2021, Intel Corporation
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
//! \file     vphal_render_fast1ton.h
//! \brief    Common interface and structure used in multi scaling output
//! \details  This module contains video surface multi scaling output definitions for VPHAL
//!

#ifndef __VPHAL_RENDER_FAST1TON_H__
#define __VPHAL_RENDER_FAST1TON_H__

#include "mos_os.h"
#include "renderhal_legacy.h"
#include "vphal_render_common.h"

#define MAX_1TON_SUPPORT 3    // currently the multi output number max support 3

// Static Data for Gen9 fast1ton kernel
typedef struct _MEDIA_WALKER_FAST1TON_STATIC_DATA
{
    // DWORD 0 - GRF R1.0
    union
    {
        uint32_t       Sampler_Index0;      // avs sampler
    } DW0;

    // DWORD 1 - GRF R1.1
    union
    {
        uint32_t       Sampler_Index1;      // avs sampler
    } DW1;

    // DWORD 2 - GRF R1.2
    union
    {
        uint32_t       Sampler_Index2;      // avs sampler
    } DW2;

    // DWORD 3 - GRF R1.3
    union
    {
        uint32_t        Src_Index;        // Input - NV12( Planar_420_8)
    } DW3;

    // DWORD 4 - GRF R1.4
    union
    {
        uint32_t        Dst_Y_Index0;     // Output surface 0 NV12 Y channel w/o 16 aligned
        uint32_t        Dst_Index0;       // Output surface 0 w/ 16 aligned
    } DW4;

    // DWORD 5 - GRF R1.5
    union
    {
        uint32_t        Dst_UV_Index0;     // Output surface 0 NV12 UV channel w/o 16 aligned
    } DW5;

    // DWORD 6 - GRF R1.6
    union
    {
        uint32_t        Dst_Y_Index1;     // Output surface 1 NV12 Y channel w/o 16 aligned
        uint32_t        Dst_Index1;       // Output surface 1 w/ 16 aligned
    } DW6;

    // DWORD 7 - GRF R1.7
    union
    {
        uint32_t        Dst_UV_Index1;     // Output surface 1 NV12 UV channel w/o 16 aligned
    } DW7;

    // DWORD 8 - GRF R2.0
    union
    {
        uint32_t        Dst_Y_Index2;     // Output surface 2 NV12 Y channel w/o 16 aligned
        uint32_t        Dst_Index2;       // Output surface 2 w/ 16 aligned
    } DW8;

    // DWORD 9 - GRF R2.0
    union
    {
        uint32_t        Dst_UV_Index2;        // Output surface 2 NV12 UV channel w/o 16 aligned
    } DW9;

    // DWORD 10 - GRF R2.0
    union
    {
        struct {
            uint32_t    Dst_16Aligned   : 16; // 1 Enable 0 Disable (Bit 0 -2 are available, bit 0 maps to Output0, bit 1 maps to output1, and bit 2 maps to output2.)
            uint32_t    Dst_pitch0 : 16;   // pitch of Output surface 0, Only available when 16 alignment is set
        };
    } DW10;

    // DWORD 11 - GRF R2.0
    union
    {
        struct {
            uint32_t    Dst_pitch1 : 16;   // pitch of Output surface 1, Only available when 16 alignment is set
            uint32_t    Dst_pitch2 : 16;   // pitch of Output surface 2, Only available when 16 alignment is set
        };
    } DW11;

    // DWORD 12 - GRF R2.0
    union
    {
        struct {
            uint32_t    Dst_UVOffset0 : 16;  // UV Offset (Lines) of output surface 0 comparing to Y (Map to 2D).
            uint32_t    Dst_UVOffset1 : 16;  // UV Offset (Lines) of output surface 1 comparing to Y (Map to 2D).
        };
    } DW12;

    // DWORD 13 - GRF R2.0
    union
    {
        struct {
            uint32_t    Dst_UVOffset2 : 16;  // UV Offset (Lines) of output surface 2 comparing to Y (Map to 2D).
            uint32_t    Reversed         : 16;
        };
    } DW13;

    // DWORD 14 - GRF R2.0
    union
    {
        float           ScalingStep_H0;       // Horizontal Scale Step for Output 0.
    } DW14;

    // DWORD 15 - GRF R2.0
    union
    {
        float           ScalingStep_V0;       // Vertical Scale Step for Output 0.
    } DW15;

    // DWORD 16 - GRF R2.0
    union
    {
        float           ScalingStep_H1;       // Horizontal Scale Step for Output 1.
    } DW16;

    // DWORD 17 - GRF R2.0
    union
    {
        float           ScalingStep_V1;       // Vertical Scale Step for Output 1.
    } DW17;

    // DWORD 18 - GRF R2.0
    union
    {
        float           ScalingStep_H2;       // Horizontal Scale Step for Output 2.
    } DW18;

    // DWORD 19 - GRF R2.0
    union
    {
        float           ScalingStep_V2;       // Vertical Scale Step for Output 2.
    } DW19;

    // DWORD 20 - GRF R2.0
    union
    {
        uint32_t        Dst_OriginalX0;     // Original X shift for surface 0
    } DW20;

    // DWORD 21 - GRF R2.0
    union
    {
        uint32_t        Dst_OriginalY0;     // Original Y shift for surface 0
    } DW21;

    // DWORD 22 - GRF R2.0
    union
    {
        uint32_t        Dst_OriginalX1;     // Original X shift for surface 1
    } DW22;

    // DWORD 23 - GRF R2.0
    union
    {
        uint32_t        Dst_OriginalY1;     // Original Y shift for surface 1
    } DW23;

    // DWORD 24 - GRF R2.0
    union
    {
        uint32_t        Dst_OriginalX2;     // Original X shift for surface 2
    } DW24;

    // DWORD 25 - GRF R2.0
    union
    {
        uint32_t        Dst_OriginalY2;     // Original Y shift for surface 2
    } DW25;
}MEDIA_WALKER_FAST1TON_STATIC_DATA, * PMEDIA_WALKER_FAST1TON_STATIC_DATA;

//!
//! \brief VPHAL fast 1toN render data
//!
typedef struct _VPHAL_FAST1TON_RENDER_DATA
{
    int32_t                             iBlocksX;
    int32_t                             iBlocksY;
    int32_t                             iBindingTable;
    int32_t                             iMediaID;
    PRENDERHAL_MEDIA_STATE              pMediaState;
    float                               ScalingStep_H[MAX_1TON_SUPPORT];
    float                               ScalingStep_V[MAX_1TON_SUPPORT];
    float                               ScalingRatio_H[MAX_1TON_SUPPORT];
    float                               ScalingRatio_V[MAX_1TON_SUPPORT];
    // Kernel Information
    PRENDERHAL_KERNEL_PARAM             pKernelParam;
    Kdll_CacheEntry                     KernelEntry;
    int32_t                             iCurbeLength;
    int32_t                             iInlineLength;

    MHW_SAMPLER_STATE_PARAM             SamplerStateParams[MAX_1TON_SUPPORT];           //!< Sampler State 
    PMHW_AVS_PARAMS                     pAVSParameters[MAX_1TON_SUPPORT];               //!< AVS parameters
    MHW_SAMPLER_AVS_TABLE_PARAM         mhwSamplerAvsTableParam[MAX_1TON_SUPPORT];      //!< params for AVS scaling 8x8 table

    // Perf
    VPHAL_PERFTAG                       PerfTag;

    // Debug parameters
    // Kernel Used for current rendering
    char*                               pKernelName;
} VPHAL_FAST1TON_RENDER_DATA, *PVPHAL_FAST1TON_RENDER_DATA;

//!
//! \brief VPHAL fast 1toN render state
//!
typedef struct _VPHAL_FAST1TON_STATE *PVPHAL_FAST1TON_STATE;
typedef struct _VPHAL_FAST1TON_STATE
{
    // External components and tables
    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_INTERFACE            pRenderHal;
    Kdll_State                      *pKernelDllState; 
    MEDIA_FEATURE_TABLE             *pSkuTable;
    MEDIA_WA_TABLE                  *pWaTable;
    bool                            bFtrMediaWalker;
    MHW_AVS_PARAMS                  AVSParameters[MAX_1TON_SUPPORT];   //!< AVS for configure Sampler
    MHW_SAMPLER_AVS_TABLE_PARAM     mhwSamplerAvsTableParam[MAX_1TON_SUPPORT];
    // Input and output surfaces
    PVPHAL_SURFACE                  pSource;
    PVPHAL_SURFACE                  pTarget[MAX_1TON_SUPPORT];
    RENDERHAL_SURFACE               RenderHalSource; // source for mhw
    RENDERHAL_SURFACE               RenderHalTarget[MAX_1TON_SUPPORT]; // target for mhw
    uint32_t                        uDstCount;
    bool                            Aligned16[MAX_1TON_SUPPORT];

    // Internal parameters
    // iBindingTableID: Binding table ID
    // SurfMemObjCtl:   Cache attributes for sampled surface and target surface
    int32_t                         iBindingTableID;
    VPHAL_FAST1TON_CACHE_CNTL       SurfMemObjCtl;

    // Platform dependent states
    PRENDERHAL_KERNEL_PARAM         pKernelParamTable;
    
    // Null rendering flag for fast 1toN function
    bool                            bNullHwRenderfast1toN;

    // Feature reporting
    VphalFeatureReport              Reporting;

    // Performance Related item
    PVPHAL_RNDR_PERF_DATA           pPerfData;

    // Status table, Vide Pre-Processing Only
    STATUS_TABLE_UPDATE_PARAMS      StatusTableUpdateParams;

    // Interface Functions
    MOS_STATUS (* pfnInitialize) (
        PVPHAL_FAST1TON_STATE        pFast1toNState,
        const VphalSettings          *pSettings,
        Kdll_State                   *pKernelDllState);

    MOS_STATUS (* pfnDestroy) (
        PVPHAL_FAST1TON_STATE         pFast1toNState);

    MOS_STATUS (* pfnRender) (
        PVPHAL_FAST1TON_STATE         pFast1toNState,
        PVPHAL_RENDER_PARAMS          pRenderParams);

    MOS_STATUS (* pfnSetupSurfaceStates) (
        PVPHAL_FAST1TON_STATE         pFast1toNState,    
        PVPHAL_FAST1TON_RENDER_DATA   pRenderData);

    MOS_STATUS (* pfnLoadStaticData) (
        PVPHAL_FAST1TON_STATE         pFast1toNState,    
        PVPHAL_FAST1TON_RENDER_DATA   pRenderData,
        int32_t                       *piCurbeOffset);

    MOS_STATUS (* pfnSetupKernel) (
        PVPHAL_FAST1TON_STATE         pFast1toNState,
        PVPHAL_FAST1TON_RENDER_DATA   pRenderData);

    MOS_STATUS (* pfnSetSamplerStates) (
        PVPHAL_FAST1TON_STATE         pFast1toNState,
        PVPHAL_FAST1TON_RENDER_DATA   pRenderData);
} VPHAL_FAST1TON_STATE;


//!
//! \brief    fast 1toN interface Initializations
//! \details  Interface Initializations for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in/out] Pointer to RenderHal Interface Structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_Fast1toNInitInterface(
    PVPHAL_FAST1TON_STATE    pFast1toNState,
    PRENDERHAL_INTERFACE     pRenderHal);

//!
//! \brief    check if intput/output is a fast 1toN case
//! \param    [in] pRenderParams
//!           Pointer to VPHAL render parameter
//! \param    [in] pSrcSurface
//!           Pointer to input surface.
//! \return   ture if this case match fas 1toN condition, otherwise return fasle.
//!
bool VpHal_RndrIsFast1toNSupport(
    PVPHAL_FAST1TON_STATE   pFast1toNState,
    PVPHAL_RENDER_PARAMS    pRenderParams,
    PVPHAL_SURFACE          pSrcSurface);
#endif // __VPHAL_RENDER_FAST1TON_H__


