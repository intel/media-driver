/*
* Copyright (c) 2022, Intel Corporation
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
//! \file      renderhal_legacy.h 
//! \brief 
//!
//!
//! \file     renderhal_legacy.h
//! \brief    Render Engine Interfaces shared across platforms
//! \details  Platform Independent Hardware Interfaces
//!
#ifndef __RENDERHAL_LEGACY_H__
#define __RENDERHAL_LEGACY_H__

#include "renderhal.h"
#include "mhw_render_legacy.h"
#include "renderhal_dsh.h"
#include "cm_hal_hashtable.h"
#include "media_perf_profiler.h"

// Forward declarations
typedef struct _RENDERHAL_INTERFACE_LEGACY  RENDERHAL_INTERFACE_LEGACY, *PRENDERHAL_INTERFACE_LEGACY;

//!
//! Structure RENDERHAL_SETTINGS_LEGACY
//! \brief RenderHal Settings - creation parameters for RenderHal
//!
typedef struct _RENDERHAL_SETTINGS_LEGACY : _RENDERHAL_SETTINGS
{
    PRENDERHAL_DYN_HEAP_SETTINGS pDynSettings = nullptr; // Dynamic State Heap Settings
} RENDERHAL_SETTINGS_LEGACY, *PRENDERHAL_SETTINGS_LEGACY;

typedef struct _RENDERHAL_MEDIA_STATE_LEGACY *PRENDERHAL_MEDIA_STATE_LEGACY;

typedef struct _RENDERHAL_DYNAMIC_STATE *PRENDERHAL_DYNAMIC_STATE;

typedef struct _RENDERHAL_MEDIA_STATE_LEGACY : _RENDERHAL_MEDIA_STATE
{
    PRENDERHAL_DYNAMIC_STATE    pDynamicState = nullptr;                     // Dynamic states (nullptr if DSH not in use)
} RENDERHAL_MEDIA_STATE_LEGACY, *PRENDERHAL_MEDIA_STATE_LEGACY;

typedef struct _RENDERHAL_STATE_HEAP_LEGACY : _RENDERHAL_STATE_HEAP
{                        
    CmHashTable                 kernelHashTable;                             // Kernel hash table for faster kernel search
} RENDERHAL_STATE_HEAP_LEGACY, *PRENDERHAL_STATE_HEAP_LEGACY;

//!
// \brief   Hardware dependent render engine interface
//!
typedef struct _RENDERHAL_INTERFACE_LEGACY : _RENDERHAL_INTERFACE
{
    // MOS/MHW Interfaces
    MhwRenderInterface            *pMhwRenderInterface = nullptr;
    RENDERHAL_DYN_HEAP_SETTINGS   DynamicHeapSettings;                       //!< Dynamic State Heap Settings

    //---------------------------
    // ISA ASM Debug support functions
    //---------------------------
    int32_t (* pfnLoadDebugKernel)(
            PRENDERHAL_INTERFACE  pRenderHal,
            PMHW_KERNEL_PARAM     pKernel);

    MOS_STATUS (* pfnLoadSipKernel) (
            PRENDERHAL_INTERFACE  pRenderHal,
            void                  *pSipKernel,
            uint32_t              dwSipSize);
    
    MOS_STATUS (* pfnSendSipStateCmd) (
            PRENDERHAL_INTERFACE  pRenderHal,
            PMOS_COMMAND_BUFFER   pCmdBuffer);
    
    void (* pfnSetupPrologParams) (
        PRENDERHAL_INTERFACE             renderHal,
        RENDERHAL_GENERIC_PROLOG_PARAMS  *prologParams,
        PMOS_RESOURCE                    osResource,
        uint32_t                         offset,
        uint32_t                         tag);

} RENDERHAL_INTERFACE_LEGACY, *PRENDERHAL_INTERFACE_LEGACY;

//!
//! \brief    Create Interface
//! \details  Create RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE renderHal
//!           [in/out] Pointer to Hardware Interface Structure
//! \param    MhwCpInterface* cpInterface
//!           [out] Pointer of pointer to MHW CP Interface Structure, which
//!           is created during renderhal initialization
//! \param    PMOS_INTERFACE osInterface
//!           [in] Pointer to OS Interface Structure
//! \param    PRENDERHAL_SETTINGS_LEGACY renderHalSettings
//!           [in] Pointer to RenderHal Settings
//!
MOS_STATUS Create_RenderHal_Interface_Legacy(
    PRENDERHAL_INTERFACE &renderHal,
    MhwCpInterface       **cpInterface,
    PMOS_INTERFACE       osInterface,
    PRENDERHAL_SETTINGS_LEGACY renderHalSettings);

//!
//! \brief    Destroy Interface
//! \details  Create RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE_LEGACY renderHal
//!           [in] Pointer to Hardware Interface Structure
//!
MOS_STATUS Destroy_RenderHal_Interface_Legacy(
    PRENDERHAL_INTERFACE_LEGACY renderHal);

//!
//! \brief    Init Interface
//! \details  Initializes Render Hal Interface structure, responsible for HW
//!           abstraction of Render Engine for MDF/VP
//! \param    PRENDERHAL_INTERFACE_LEGACY pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    MhwCpInterface** ppCpInterface
//!           [in/out] Pointer of pointer to MHW CP Interface Structure, which 
//!           is created during renderhal initialization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_UNKNOWN : Invalid parameters
//!
MOS_STATUS RenderHal_InitInterface_Legacy(
    PRENDERHAL_INTERFACE_LEGACY pRenderHal,
    MhwCpInterface              **ppCpInterface,
    PMOS_INTERFACE              pOsInterface);

//!
//! \brief    Init Interface using Dynamic State Heap
//! \details  Initializes RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE_LEGACY pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    MhwCpInterface** ppCpInterface
//!           [in/out] Pointer of pointer to MHW CP Interface Structure, which 
//!           is created during renderhal initialization
//! \param    PMOS_INTERFACE pOsInterface
//!           [in] Pointer to OS Interface Structure
//! \return   MOS_STATUS
//!           MOS_STATUS_UNKNOWN : Invalid parameters
//!
MOS_STATUS RenderHal_InitInterface_Dynamic(
    PRENDERHAL_INTERFACE_LEGACY pRenderHal,
    MhwCpInterface              **ppCpInterface,
    PMOS_INTERFACE              pOsInterface);

//!
//! \brief    Init Special Interface
//! \details  Initializes RenderHal Interface structure, responsible for HW
//!           abstraction of HW Rendering Engine for CM(MDF) and VP.
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//!
void RenderHal_InitInterfaceEx_Legacy(
    PRENDERHAL_INTERFACE_LEGACY    pRenderHal);

int32_t RenderHal_LoadDebugKernel(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMHW_KERNEL_PARAM       pSipKernel);

MOS_STATUS RenderHal_LoadSipKernel(
    PRENDERHAL_INTERFACE    pRenderHal,
    void                    *pSipKernel,
    uint32_t                dwSipSize);

MOS_STATUS RenderHal_SendSipStateCmd(
    PRENDERHAL_INTERFACE    pRenderHal,
    PMOS_COMMAND_BUFFER     pCmdBuffer);

void RenderHal_SetupPrologParams(
    PRENDERHAL_INTERFACE              renderHal,
    RENDERHAL_GENERIC_PROLOG_PARAMS  *prologParams,
    PMOS_RESOURCE                     osResource,
    uint32_t                          offset,
    uint32_t                          tag);

#endif // __RENDERHAL_LEGACY_H__
