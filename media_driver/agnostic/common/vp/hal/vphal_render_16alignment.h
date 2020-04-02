/*
*
* Copyright (c) Intel Corporation 2018.
*
* INTEL MAKES NO WARRANTY OF ANY KIND REGARDING THE CODE.  THIS CODE IS
* LICENSED ON AN "AS IS" BASIS AND INTEL WILL NOT PROVIDE ANY SUPPORT,
* ASSISTANCE, INSTALLATION, TRAINING OR OTHER SERVICES.  INTEL DOES NOT
* PROVIDE ANY UPDATES, ENHANCEMENTS OR EXTENSIONS.  INTEL SPECIFICALLY
* DISCLAIMS ANY WARRANTY OF MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR ANY
* PARTICULAR PURPOSE, OR ANY OTHER WARRANTY.  Intel disclaims all liability,
* including liability for infringement of any proprietary rights, relating to
* use of the code. No license, express or implied, by estoppel or otherwise,
* to any intellectual property rights is granted herein.
*
*
* File Name  : vphal_render_16alignment.h
*
* Abstract   : Video surface 16 bytes alignment definitions for Video Processing
*
* Environment: Linux
*
* Notes      : This module contains video surface 16 bytes alignment definitions
*              for VPHAL
*
*/
//!
//! \file     vphal_render_16alignment.h
//! \brief    Common interface and structure used in 16align
//! \details  Common interface and structure used in 16align
//!
#ifndef __VPHAL_RENDER_16ALIGNMENT_H__
#define __VPHAL_RENDER_16ALIGNMENT_H__

#include "mos_os.h"
#include "renderhal.h"
#include "vphal_render_common.h"

// Static Data for Gen9 16ALIGN kernel
typedef struct _MEDIA_WALKER_16ALIGN_STATIC_DATA
{
    // DWORD 0 - GRF R1.0
    union
    {
        uint32_t       Sampler_Index;
    } DW0;

    // DWORD 1 - GRF R1.1
    union
    {
        uint32_t       pSrcSurface;          // Input - YUY2/ARGB
        uint32_t       pSrcSurface_Y;        // Input - Y Channel for NV12/YV12
    } DW1;

    // DWORD 2 - GRF R1.2
    union
    {
        uint32_t        pSrcSurface_UV;       // Input - UV Channel for NV12
        uint32_t        pSrcSurface_U;        // Input - U Channel for YV12
    } DW2;

    // DWORD 3 - GRF R1.3
    union
    {
        uint32_t        pSrcSurface_V;        // Input - V Channel for YV12
    } DW3;

    // DWORD 4 - GRF R1.4
    union
    {
        uint32_t        pOutSurface;          // Output - Buffer YUY2, size need to be 16 bytes alignment.
        uint32_t        pOutSurface_Y;        // Output - Buffer NV12/YV12, size need to be 32 bytes alignment
    } DW4;

    // DWORD 5 - GRF R1.5
    union
    {
        uint32_t        pOutSurface_UV;       // Output - Buffer NV12, size need to be 16 bytes alignment.
        uint32_t        pOutSurface_U;        // Output - Buffer YV12, size need to be 16 bytes alignment.
    } DW5;

    // DWORD 6 - GRF R1.6
    union
    {
        uint32_t        pOutSurface_V;        // Output - Buffer YV12, size need to be 16 bytes alignment.
    } DW6;

    // DWORD 7 - GRF R1.7
    union
    {
        float           ScalingStep_H;       // Scaling ratio in Horizontal direction.
    } DW7;

    // DWORD 8 - GRF R2.0
    union
    {
        float           ScalingStep_V;       // Scaling ratio in Vertical direction.
    } DW8;

    // DWORD 9 - GRF R2.1
    union
    {
        struct {
            uint32_t       Input_Format    : 16;  // 0 NV12, 1 YUY2, 2 YV12, 3 argb
            uint32_t       Output_Format   : 16;  // 0 NV12, 1 YUY2, 2 YV12
        };
    } DW9;

    // DWORD 10 - GRF R2.2
    union
    {
        struct {
            uint32_t       Output_Pitch    : 16;  // Pitch of Output surface (Map to 2D)
            uint32_t       Output_Height   : 16;  // the Height of output surface
        };
    } DW10;

    // DWORD 11 - GRF R2.3
    union
    {
        uint32_t        ScalingMode;              // 0 for 3D(bilinear, Nearest), 1 for AVS
    } DW11;

    // DWORD 12 - GRF R2.4
    union
    {
        float           Original_X;
    } DW12;

    // DWORD 13 - GRG R2.5
    union
    {
        float           Original_Y;
    } DW13;

    // DWORD 14 - GRG R2.6
    union
    {
        uint32_t        reserved;
    } DW14;

    // DWORD 15 - GRG R2.7
    union
    {
        uint32_t        reserved;
    } DW15;

    // DWORD 16 - GRG R3.0
    union
    {
        struct {
            uint32_t       CSC_COEFF_0    : 16;
            uint32_t       CSC_COEFF_1    : 16;
        };
    } DW16;

    // DWORD 17 - GRG R3.1
    union
    {
        struct {
            uint32_t       CSC_COEFF_2   : 16;
            uint32_t       CSC_COEFF_3   : 16;
        };
    } DW17;

    // DWORD 18 - GRG R3.2
    union
    {
        struct {
            uint32_t       CSC_COEFF_4   : 16;
            uint32_t       CSC_COEFF_5   : 16;
        };
    } DW18;

    // DWORD 19 - GRG R3.3
    union
    {
        struct {
            uint32_t       CSC_COEFF_6   : 16;
            uint32_t       CSC_COEFF_7   : 16;
        };
    } DW19;

    // DWORD 20 - GRG R3.4
    union
    {
        struct {
            uint32_t       CSC_COEFF_8   : 16;
            uint32_t       CSC_COEFF_9   : 16;
        };
    } DW20;

    // DWORD 21 - GRG R3.5
    union
    {
        struct {
            uint32_t       CSC_COEFF_10   : 16;
            uint32_t       CSC_COEFF_11   : 16;
        };
    } DW21;

    // DWORD 22 - GRG R3.6
    union
    {
        struct {
            uint32_t       OutputMode     :  1;   // 0 UsrPtr, 1: VA
            uint32_t       reserved       : 15;
            uint32_t       Output_Top     : 16;   // output cropping Top
        };
    } DW22;

    // DWORD 23 - GRG R3.7
    union
    {
        struct {
            uint32_t       Output_Bottom  : 16;   // output cropping Bottom
            uint32_t       Output_Left    : 16;   // output cropping left
        };
    } DW23;

    // DWORD 24 - GRG R4.0
    union
    {
        struct {
            uint32_t       Output_Right   : 16;    // output cropping right
            uint32_t       bClearFlag     : 1;     // 0 don't clear out of cropped area, 1 Clear unavaliable area
            uint32_t       reserved       : 15;
        };
    } DW24;
}MEDIA_WALKER_16ALIGN_STATIC_DATA, * PMEDIA_WALKER_16ALIGN_STATIC_DATA;

//!
//! \brief VPHAL 16 Bytes Alignment render data
//!
typedef struct _VPHAL_16_ALIGN_RENDER_DATA
{
    int32_t                             iBlocksX;
    int32_t                             iBlocksY;
    int32_t                             iBindingTable;
    int32_t                             iMediaID;
    PRENDERHAL_MEDIA_STATE              pMediaState;
    float                               ScalingRatio_H;
    float                               ScalingRatio_V;
    // Kernel Information
    PRENDERHAL_KERNEL_PARAM             pKernelParam;
    Kdll_CacheEntry                     KernelEntry;
    int32_t                             iCurbeLength;
    int32_t                             iInlineLength;
    uint32_t                            dwSurfStateWd;       //!< Surface Height as programmed in SS
    uint32_t                            dwSurfStateHt;       //!< Surface Height as programmed in SS

    MHW_SAMPLER_STATE_PARAM             SamplerStateParams;           //!< Sampler State 
    PMHW_AVS_PARAMS                     pAVSParameters;               //!< AVS parameters
    MHW_SAMPLER_AVS_TABLE_PARAM         mhwSamplerAvsTableParam;      //!< params for AVS scaling 8x8 table

    // Perf
    VPHAL_PERFTAG                       PerfTag;

    // Debug parameters
    // Kernel Used for current rendering
    char*                               pKernelName;
} VPHAL_16_ALIGN_RENDER_DATA, *PVPHAL_16_ALIGN_RENDER_DATA;

//!
//! \brief VPHAL 16 Bytes Alignment render state
//!
typedef struct _VPHAL_16_ALIGN_STATE *PVPHAL_16_ALIGN_STATE;
typedef struct _VPHAL_16_ALIGN_STATE
{
    // External components and tables
    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_INTERFACE            pRenderHal;
    Kdll_State                      *pKernelDllState; 
    MEDIA_FEATURE_TABLE             *pSkuTable;
    MEDIA_WA_TABLE                  *pWaTable;
    bool                            bFtrMediaWalker;
    MHW_AVS_PARAMS                  AVSParameters;                  //!< AVS for configure Sampler
    MHW_SAMPLER_AVS_TABLE_PARAM     mhwSamplerAvsTableParam;
    // Input and output surfaces
    PVPHAL_SURFACE                  pSource;
    PVPHAL_SURFACE                  pTarget;
    RENDERHAL_SURFACE               RenderHalSource; // source for mhw
    RENDERHAL_SURFACE               RenderHalTarget; // target for mhw

    // Internal parameters
    // iBindingTableID: Binding table ID
    // SurfMemObjCtl:   Cache attributes for sampled surface and target surface
    int32_t                         iBindingTableID;
    VPHAL_16_ALIGN_CACHE_CNTL       SurfMemObjCtl;

    // Platform dependent states
    PRENDERHAL_KERNEL_PARAM         pKernelParamTable;
    
    // Null rendering flag for 16 Bytes Alignment function
    bool                            bNullHwRender16Align;

    // Feature reporting
    VphalFeatureReport              Reporting;

    // Performance Related item
    PVPHAL_RNDR_PERF_DATA           pPerfData;

    // Status table, Vide Pre-Processing Only
    STATUS_TABLE_UPDATE_PARAMS      StatusTableUpdateParams;

    // Interface Functions
    MOS_STATUS (* pfnInitialize) (
        PVPHAL_16_ALIGN_STATE        p16AlignState,
        const VphalSettings          *pSettings,
        Kdll_State                   *pKernelDllState);

    MOS_STATUS (* pfnDestroy) (
        PVPHAL_16_ALIGN_STATE         p16AlignState);

    MOS_STATUS (* pfnRender) (
        PVPHAL_16_ALIGN_STATE         p16AlignState,
        PVPHAL_RENDER_PARAMS          pRenderParams);

    MOS_STATUS (* pfnSetupSurfaceStates) (
        PVPHAL_16_ALIGN_STATE         p16AlignState,    
        PVPHAL_16_ALIGN_RENDER_DATA   pRenderData);

    MOS_STATUS (* pfnLoadStaticData) (
        PVPHAL_16_ALIGN_STATE         p16AlignState,    
        PVPHAL_16_ALIGN_RENDER_DATA   pRenderData,
        int32_t                       *piCurbeOffset);

    MOS_STATUS (* pfnSetupKernel) (
        PVPHAL_16_ALIGN_STATE         p16AlignState,
        PVPHAL_16_ALIGN_RENDER_DATA   pRenderData);

    MOS_STATUS (* pfnSetSamplerStates) (
        PVPHAL_16_ALIGN_STATE         p16AlignState,
        PVPHAL_16_ALIGN_RENDER_DATA   pRenderData);
} VPHAL_16_ALIGN_STATE;


//!
//! \brief    16Align interface Initializations
//! \details  Interface Initializations for 16Align
//! \param    PVPHAL_16_ALIGN_STATE p16AlignState
//!           [in] Pointer to the 16Align State
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in/out] Pointer to RenderHal Interface Structure
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VpHal_16AlignInitInterface(
    PVPHAL_16_ALIGN_STATE    p16AlignState,
    PRENDERHAL_INTERFACE     pRenderHal);

//!
//! \brief    check 16Align whether can be processed
//! \details  check 16Align whether can be processed
//! \param    PVPHAL_RENDER_PARAMS  pRenderParams
//!           [in] Pointer to VPHAL render parameter
//! \return   bool
//!           Return true if 16 Bytes Alignment can be processed, otherwise false
//!
bool VpHal_RndrIs16Align(
    PVPHAL_16_ALIGN_STATE   p16AlignState,
    PVPHAL_RENDER_PARAMS    pRenderParams);
#endif // __VPHAL_RENDER_16ALIGNMENT_H__

