/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020, Intel Corporation
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file       renderhal_xe_hpg.cpp
//! \brief      implementation of Gen12_7 hardware functions
//! \details    Render functions
//!

#include "renderhal_legacy.h"
#include "renderhal_xe_hpg.h"
#include "vp_utils.h"
#include "mhw_render_xe_hp_base.h"
#include "mhw_state_heap.h"
#include "mos_os.h"
#include "mos_utilities.h"
#include "mos_utilities_common.h"
#include "vp_common.h"

#define RENDERHAL_SAMPLERS_AVS_HPG 0
#define ENLARGE_KERNEL_COUNT_HPG   RENDERHAL_KERNEL_COUNT * 3
#define ENLARGE_KERNEL_HEAP_HPG    RENDERHAL_KERNEL_HEAP * 3
#define ENLARGE_CURBE_SIZE_HPG     RENDERHAL_CURBE_SIZE * 16

extern const RENDERHAL_STATE_HEAP_SETTINGS g_cRenderHal_State_Heap_Settings_xe_hpg =
{
    // Global GSH Allocation parameters
    RENDERHAL_SYNC_SIZE,                       //!< iSyncSize

    // Media State Allocation parameters
    RENDERHAL_MEDIA_STATES,                    //!< iMediaStateHeaps - Set by Initialize
    RENDERHAL_MEDIA_IDS,                       //!< iMediaIDs
    RENDERHAL_CURBE_SIZE,                      //!< iCurbeSize
    RENDERHAL_SAMPLERS,                        //!< iSamplers
    RENDERHAL_SAMPLERS_AVS_HPG,                //!< iSamplersAVS
    RENDERHAL_SAMPLERS_VA,                     //!< iSamplersVA
    RENDERHAL_KERNEL_COUNT,                    //!< iKernelCount
    RENDERHAL_KERNEL_HEAP,                     //!< iKernelHeapSize
    RENDERHAL_KERNEL_BLOCK_SIZE,               //!< iKernelBlockSize

    // Media VFE/ID configuration, limits
    0,                                         //!< iPerThreadScratchSize
    RENDERHAL_MAX_SIP_SIZE,                    //!< iSipSize

    // Surface State Heap Settings
    RENDERHAL_SSH_INSTANCES,                   //!< iSurfaceStateHeaps
    RENDERHAL_SSH_BINDING_TABLES,              //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES,              //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT,             //!< iSurfacesPerBT
    RENDERHAL_SSH_BINDING_TABLE_ALIGN,         //!< iBTAlignment
    MOS_CODEC_RESOURCE_USAGE_BEGIN_CODEC       //!< heapUsageType
};

extern const RENDERHAL_ENLARGE_PARAMS g_cRenderHal_Enlarge_State_Heap_Settings_Adv_xe_hpg =
{
    RENDERHAL_SSH_BINDING_TABLES_MAX,  //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES_MAX,  //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT,      //!< iSurfacesPerBT
    ENLARGE_KERNEL_COUNT_HPG,          //!< iKernelCount
    ENLARGE_KERNEL_HEAP_HPG,           //!< iKernelHeapSize
    ENLARGE_CURBE_SIZE_HPG             //!< iCurbeSize
};

MOS_STATUS XRenderHal_Interface_Xe_Hpg::IsRenderHalMMCEnabled(
    PRENDERHAL_INTERFACE pRenderHal)
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;

    MHW_RENDERHAL_CHK_NULL_NO_STATUS(pRenderHal);

    // Read user feature key to set MMC for Fast Composition surfaces
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#if defined(LINUX) && (!defined(WDDM_LINUX))
    UserFeatureData.bData = !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableVPMmc) || !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableCodecMmc);
#else
    UserFeatureData.bData = true;  // turn on MMC for DG2
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC_ID,
        &UserFeatureData,
        pRenderHal->pOsInterface ? pRenderHal->pOsInterface->pOsContext : nullptr));
#endif

    m_renderHalMMCEnabled    = UserFeatureData.bData && MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrE2ECompression);
    pRenderHal->isMMCEnabled = m_renderHalMMCEnabled;

finish:
    return eStatus;
}

MOS_STATUS XRenderHal_Interface_Xe_Hpg::SendComputeWalker(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer,
    PMHW_GPGPU_WALKER_PARAMS    pGpGpuWalkerParams)
{
    VP_FUNC_CALL();

    MhwRenderInterface          *pMhwRender;
    MOS_STATUS                  eStatus;
    MHW_ID_ENTRY_PARAMS         mhwIdEntryParams;
    PRENDERHAL_KRN_ALLOCATION   pKernelEntry;
    PRENDERHAL_MEDIA_STATE      pCurMediaState;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;

    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);
    MHW_RENDERHAL_CHK_NULL(pGpGpuWalkerParams);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pStateHeap->pKernelAllocation);

    MOS_ZeroMemory(&mhwIdEntryParams, sizeof(mhwIdEntryParams));

    pKernelEntry = &pRenderHalLegacy->pStateHeap->pKernelAllocation[pRenderHalLegacy->iKernelAllocationID];
    pCurMediaState = pRenderHalLegacy->pStateHeap->pCurMediaState;

    MHW_RENDERHAL_CHK_NULL(pKernelEntry);
    MHW_RENDERHAL_CHK_NULL(pCurMediaState);

    mhwIdEntryParams.dwKernelOffset = pKernelEntry->dwOffset;
    mhwIdEntryParams.dwSamplerCount = pKernelEntry->Params.Sampler_Count;
    mhwIdEntryParams.dwSamplerOffset = pCurMediaState->dwOffset +
                                        pRenderHalLegacy->pStateHeap->dwOffsetSampler +
                                        pGpGpuWalkerParams->InterfaceDescriptorOffset * pRenderHalLegacy->pStateHeap->dwSizeSampler;
    mhwIdEntryParams.dwBindingTableOffset = pGpGpuWalkerParams->BindingTableID * pRenderHalLegacy->pStateHeap->iBindingTableSize;
    mhwIdEntryParams.dwSharedLocalMemorySize = pGpGpuWalkerParams->SLMSize;
    mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup = pGpGpuWalkerParams->ThreadWidth * pGpGpuWalkerParams->ThreadHeight;
    //This only a WA to disable EU fusion for multi-layer blending cases.
    //Need remove it after kernel and compiler fix it.
    mhwIdEntryParams.bBarrierEnable = pRenderHalLegacy->eufusionBypass ? 1: 0;
    pGpGpuWalkerParams->IndirectDataStartAddress = pGpGpuWalkerParams->IndirectDataStartAddress + pRenderHalLegacy->pStateHeap->pCurMediaState->dwOffset;

    MHW_RENDERHAL_CHK_STATUS(static_cast<MhwRenderInterfaceXe_Xpm_Base*>(pRenderHalLegacy->pMhwRenderInterface)->AddComputeWalkerCmd(pCmdBuffer,
        pGpGpuWalkerParams,
        &mhwIdEntryParams,
        nullptr,
        0));

finish:
    return eStatus;
}

//!
//! \brief    Initialize the State Heap Settings per platform
//! \param    PRENDERHAL_STATE_HEAP_SETTINGS pSettings
//!           [out] Pointer to PRENDERHAL_STATE_HEAP_SETTINGSStructure
//! \return   void
//!
void XRenderHal_Interface_Xe_Hpg::InitStateHeapSettings(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set State Heap settings for xe hpg
    pRenderHal->StateHeapSettings              = g_cRenderHal_State_Heap_Settings_xe_hpg;
    pRenderHal->enlargeStateHeapSettingsForAdv = g_cRenderHal_Enlarge_State_Heap_Settings_Adv_xe_hpg;
}
