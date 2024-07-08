/*===================== begin_copyright_notice ==================================

* Copyright (c) 2024, Intel Corporation
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

======================= end_copyright_notice ==================================*/
//!
//! \file       renderhal_xe2_hpg_next.cpp
//! \brief      implementation of Gen12_9 hardware functions
//! \details    Render functions
//!

#include "renderhal.h"
#include "renderhal_xe2_hpg_next.h"
#include "vp_utils.h"
#include "media_feature.h"
#include "media_packet.h"

//!
//! \brief      GSH settings for Xe2_hpg
//!
#define RENDERHAL_SAMPLERS_AVS_XE2_HPG        0
#define RENDERHAL_SSH_SURFACES_PER_BT_XE2_HPG 80
#define ENLARGE_KERNEL_COUNT_XE2_HPG RENDERHAL_KERNEL_COUNT * 3
#define ENLARGE_KERNEL_HEAP_XE2_HPG RENDERHAL_KERNEL_HEAP * 3
#define ENLARGE_CURBE_SIZE_XE2_HPG RENDERHAL_CURBE_SIZE * 16

extern const RENDERHAL_STATE_HEAP_SETTINGS g_cRenderHal_State_Heap_Settings_xe2_hpg =
{
    // Global GSH Allocation parameters
    RENDERHAL_SYNC_SIZE,  //!< iSyncSize

    // Media State Allocation parameters
    RENDERHAL_MEDIA_STATES,           //!< iMediaStateHeaps - Set by Initialize
    RENDERHAL_MEDIA_IDS,              //!< iMediaIDs
    RENDERHAL_CURBE_SIZE,             //!< iCurbeSize
    RENDERHAL_SAMPLERS,               //!< iSamplers
    RENDERHAL_SAMPLERS_AVS_XE2_HPG,   //!< iSamplersAVS
    RENDERHAL_SAMPLERS_VA,            //!< iSamplersVA
    RENDERHAL_KERNEL_COUNT,           //!< iKernelCount
    RENDERHAL_KERNEL_HEAP,            //!< iKernelHeapSize
    RENDERHAL_KERNEL_BLOCK_SIZE,      //!< iKernelBlockSize

    // Media VFE/ID configuration, limits
    0,                       //!< iPerThreadScratchSize
    RENDERHAL_MAX_SIP_SIZE,  //!< iSipSize

    // Surface State Heap Settings
    RENDERHAL_SSH_INSTANCES,           //!< iSurfaceStateHeaps
    RENDERHAL_SSH_BINDING_TABLES,      //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES,      //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT,     //!< iSurfacesPerBT
    RENDERHAL_SSH_BINDING_TABLE_ALIGN,  //!< iBTAlignment
    MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC //!< heapUsageType
};

extern const RENDERHAL_ENLARGE_PARAMS g_cRenderHal_Enlarge_State_Heap_Settings_Adv_xe2_hpg =
{
    RENDERHAL_SSH_BINDING_TABLES_MAX,      //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES_MAX,      //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT_XE2_HPG, //!< iSurfacesPerBT
    ENLARGE_KERNEL_COUNT_XE2_HPG,          //!< iKernelCount
    ENLARGE_KERNEL_HEAP_XE2_HPG,           //!< iKernelHeapSize
    ENLARGE_CURBE_SIZE_XE2_HPG             //!< iCurbeSize
};

MOS_STATUS XRenderHal_Interface_Xe2_Hpg_Next::IsRenderHalMMCEnabled(
    PRENDERHAL_INTERFACE pRenderHal)
{
    VP_FUNC_CALL();

    MOS_STATUS   eStatus      = MOS_STATUS_SUCCESS;
    bool         isMMCEnabled = false;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);

    // Read user feature key to set MMC for Fast Composition surfaces
#if defined(LINUX) && (!defined(WDDM_LINUX))
    isMMCEnabled = !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableVPMmc) || !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableCodecMmc);
#else
    isMMCEnabled = true;  // turn on MMC
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    if (pRenderHal->userSettingPtr != nullptr)
    {
        // Read reg key to set MMC for Fast Composition surfaces
        ReadUserSettingForDebug(
            pRenderHal->userSettingPtr,
            isMMCEnabled,
            __MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC,
            MediaUserSetting::Group::Device,
            isMMCEnabled,
            true);
    }
#endif
    m_renderHalMMCEnabled    = isMMCEnabled && MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrE2ECompression);
    pRenderHal->isMMCEnabled = m_renderHalMMCEnabled;

    return eStatus;
}

//! \brief    Send To 3DState Binding Table Pool Alloc
//! \details    Send To 3DState Binding Table Pool Alloc
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!            [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!            [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Interface_Xe2_Hpg_Next::SendTo3DStateBindingTablePoolAlloc(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS                                                    eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    auto &computerModeParams          = m_renderItf->MHW_GETPAR_F(STATE_COMPUTE_MODE)();
    computerModeParams                = {};
    computerModeParams.enableLargeGrf = true;
    computerModeParams.forceEuThreadSchedulingMode = pRenderHal->euThreadSchedulingMode;
    m_renderItf->MHW_ADDCMD_F(STATE_COMPUTE_MODE)(pCmdBuffer);

    SETPAR_AND_ADDCMD(_3DSTATE_BINDING_TABLE_POOL_ALLOC, m_renderItf, pCmdBuffer);

    return eStatus;
}

//!
//! \brief    Initialize the State Heap Settings per platform
//! \param    PRENDERHAL_STATE_HEAP_SETTINGS pSettings
//!           [out] Pointer to PRENDERHAL_STATE_HEAP_SETTINGSStructure
//! \return   void
//!
void XRenderHal_Interface_Xe2_Hpg_Next::InitStateHeapSettings(
    PRENDERHAL_INTERFACE pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set State Heap settings for hpg_base
    pRenderHal->StateHeapSettings              = g_cRenderHal_State_Heap_Settings_xe2_hpg;
    pRenderHal->enlargeStateHeapSettingsForAdv = g_cRenderHal_Enlarge_State_Heap_Settings_Adv_xe2_hpg;
}