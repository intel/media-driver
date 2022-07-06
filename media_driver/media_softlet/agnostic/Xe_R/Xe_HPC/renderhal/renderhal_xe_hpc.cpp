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
//! \file       renderhal_xe_hpc.cpp
//! \brief      implementation of Gen12_8 hardware functions
//! \details    Render functions
//!

#include "renderhal_legacy.h"
#include "renderhal_xe_hpc.h"
#include "vp_utils.h"
#include "mhw_render_hwcmd_xe_hp_base.h"
#include "mhw_render_xe_hp_base.h"
#include "mhw_render_xe_hpc.h"
#include "mhw_state_heap.h"
#include "mhw_utilities_next.h"
#include "mos_resource_defs.h"
#include "mos_utilities.h"
#include "mos_utilities_common.h"
#include "vp_common.h"

//!
//! \brief    Send Compute Walker
//! \details  Send Compute Walker
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \param    PRENDERHAL_GPGPU_WALKER_PARAMS pGpGpuWalkerParams
//!           [in]    Pointer to GPGPU walker parameters
//! \return   MOS_STATUS
//!
MOS_STATUS XRenderHal_Interface_Xe_Hpc::SendComputeWalker(
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
    // PVC don't have sampler, so the sampler Count should be set as 0;
    mhwIdEntryParams.dwSamplerCount = 0;
    mhwIdEntryParams.dwSamplerOffset = pCurMediaState->dwOffset +
                                        pRenderHalLegacy->pStateHeap->dwOffsetSampler +
                                        pGpGpuWalkerParams->InterfaceDescriptorOffset * pRenderHalLegacy->pStateHeap->dwSizeSampler;
    mhwIdEntryParams.dwBindingTableOffset = pGpGpuWalkerParams->BindingTableID * pRenderHalLegacy->pStateHeap->iBindingTableSize;
    mhwIdEntryParams.dwSharedLocalMemorySize = pGpGpuWalkerParams->SLMSize;
    mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup = pGpGpuWalkerParams->ThreadWidth * pGpGpuWalkerParams->ThreadHeight;
    pGpGpuWalkerParams->IndirectDataStartAddress = pGpGpuWalkerParams->IndirectDataStartAddress + pRenderHalLegacy->pStateHeap->pCurMediaState->dwOffset;

    // prepare postsync resource;
    if (Mos_ResourceIsNull(&m_PostSyncBuffer))
    {
        MOS_ALLOC_GFXRES_PARAMS     AllocParams;
        // Allocate Predication buffer
        MOS_ZeroMemory(&AllocParams, sizeof(AllocParams));
        AllocParams.Type = MOS_GFXRES_BUFFER;
        AllocParams.TileType = MOS_TILE_LINEAR;
        AllocParams.Format = Format_Buffer;
        AllocParams.dwBytes = MHW_PAGE_SIZE;
        AllocParams.pBufName = "PostSyncBuffer";

        MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pOsInterface);

        m_pOsInterface = pRenderHalLegacy->pOsInterface;
        MHW_RENDERHAL_CHK_STATUS(m_pOsInterface->pfnAllocateResource(
            m_pOsInterface,
            &AllocParams,
            &m_PostSyncBuffer));
    }

    MHW_RENDERHAL_CHK_STATUS(static_cast<MhwRenderInterfaceXe_Hpc*>(pRenderHalLegacy->pMhwRenderInterface)->AddComputeWalkerCmd(pCmdBuffer,
        pGpGpuWalkerParams,
        &mhwIdEntryParams,
        &m_PostSyncBuffer,
        0));

finish:
    return eStatus;
}

//! \brief    Send To 3DState Binding Table Pool Alloc
//! \details    Send To 3DState Binding Table Pool Alloc
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!            [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!            [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Interface_Xe_Hpc::SendTo3DStateBindingTablePoolAlloc(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    mhw_render_xe_xpm_base::_3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD cmd;
    MHW_STATE_COMPUTE_MODE_PARAMS compute_mode_state;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;

    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);


    compute_mode_state.enableLargeGrf = true;
    eStatus = static_cast<MhwRenderInterfaceXe_Hpc*>(pRenderHalLegacy->pMhwRenderInterface)->AddStateComputeModeCmd(compute_mode_state, pCmdBuffer);

    MHW_RENDERHAL_CHK_STATUS(static_cast<MhwRenderInterfaceXe_Hpc*>(pRenderHalLegacy->pMhwRenderInterface)->Add3DStateBindingTablePoolAllocCmd(pCmdBuffer, cmd));

finish:
    return eStatus;
}

XRenderHal_Interface_Xe_Hpc::~XRenderHal_Interface_Xe_Hpc()
{
    // Release PostSyncBuffer
    if ((!Mos_ResourceIsNull(&m_PostSyncBuffer)) && (m_pOsInterface !=nullptr))
    {
        m_pOsInterface->pfnFreeResource(
            m_pOsInterface,
            &m_PostSyncBuffer);
    }
}

MOS_STATUS XRenderHal_Interface_Xe_Hpc::IsRenderHalMMCEnabled(PRENDERHAL_INTERFACE pRenderHal)
{
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA     UserFeatureData;

    MHW_RENDERHAL_CHK_NULL_NO_STATUS(pRenderHal);

    // Read user feature key to set MMC for Fast Composition surfaces
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#if defined(LINUX) && (!defined(WDDM_LINUX))
    UserFeatureData.bData = !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableVPMmc) || !MEDIA_IS_WA(pRenderHal->pWaTable, WaDisableCodecMmc);
#else
    UserFeatureData.bData = false; // init as default value to disable MMCD
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC_ID,
        &UserFeatureData,
        pRenderHal->pOsInterface ? pRenderHal->pOsInterface->pOsContext : nullptr));
#endif

    m_renderHalMMCEnabled = UserFeatureData.bData && MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrE2ECompression);
    pRenderHal->isMMCEnabled = m_renderHalMMCEnabled;

finish:
    return eStatus;
}