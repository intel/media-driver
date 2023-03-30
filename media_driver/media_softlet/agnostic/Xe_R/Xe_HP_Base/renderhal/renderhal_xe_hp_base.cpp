/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file       renderhal_xe_hp_base.cpp
//! \brief      implementation of Gen12_5+ hardware functions
//! \details    Render functions
//!

#include "renderhal_legacy.h"
#include "media_skuwa_specific.h"
#include "mhw_render_g12_X.h"
#include "mhw_render_hwcmd_xe_hp_base.h"
#include "mhw_render_xe_hp_base.h"
#include "mhw_state_heap.h"
#include "mhw_utilities_next.h"
#include "mos_os.h"
#include "mos_resource_defs.h"
#include "mos_utilities.h"
#include "mos_utilities_common.h"
#include "vp_common.h"
#include "renderhal.h"
#include "renderhal_xe_hp_base.h"
#include "vp_utils.h"

MOS_STATUS XRenderHal_Interface_Xe_Hp_Base::IsRenderHalMMCEnabled(
    PRENDERHAL_INTERFACE         pRenderHal)
{
    VP_FUNC_CALL();

    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA           UserFeatureData;

    MHW_RENDERHAL_CHK_NULL_NO_STATUS(pRenderHal);

    // Read user feature key to set MMC for Fast Composition surfaces
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#ifdef LINUX
    UserFeatureData.bData = false;
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

MOS_STATUS XRenderHal_Interface_Xe_Hp_Base::SetupSurfaceState(
    PRENDERHAL_INTERFACE             pRenderHal,
    PRENDERHAL_SURFACE               pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS  pParams,
    int32_t                          *piNumEntries,
    PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
    PRENDERHAL_OFFSET_OVERRIDE       pOffsetOverride)
{
    VP_FUNC_CALL();

    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntry;
    PMOS_PLANE_OFFSET               pPlaneOffset;
    MHW_SURFACE_STATE_PARAMS        SurfStateParams;
    PMOS_SURFACE                    pSurface;
    int32_t                         i;
    uint32_t                        dwPixelsPerSampleUV;
    uint32_t                        dwSurfaceSize;
    MOS_STATUS                      eStatus = MOS_STATUS_UNKNOWN;

    //-----------------------------------------
    MHW_RENDERHAL_UNUSED(pOffsetOverride);
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL(pParams);
    MHW_RENDERHAL_CHK_NULL(ppSurfaceEntries);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_ASSERT(pRenderHalSurface->Rotation >= MHW_ROTATION_IDENTITY && pRenderHalSurface->Rotation <= MHW_ROTATE_90_MIRROR_HORIZONTAL);
    //-----------------------------------------

    dwSurfaceSize = pRenderHal->pHwSizes->dwSizeSurfaceState;

    MOS_ZeroMemory(&SurfStateParams, sizeof(SurfStateParams));

    // Get the Surface State Entries
    MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnGetSurfaceStateEntries(
            pRenderHal,
            pRenderHalSurface,
            pParams,
            piNumEntries,
            ppSurfaceEntries));

    for (i = 0; i < *piNumEntries; i++)
    {
        // Pointer to surface state entry for current plane
        pSurfaceEntry = ppSurfaceEntries[i];

        pSurface = pSurfaceEntry->pSurface;
        MHW_RENDERHAL_CHK_NULL(pSurface);

        // Set the Surface State Offset from base of SSH
        pSurfaceEntry->dwSurfStateOffset = pRenderHal->pStateHeap->iSurfaceStateOffset +                // Offset to Base Of Current Surface State Area
                                           pSurfaceEntry->iSurfStateID * dwSurfaceSize;                 // Offset  to Surface State within the area

        // Obtain the Pointer to the Surface state from SSH Buffer
        SurfStateParams.pSurfaceState         = pSurfaceEntry->pSurfaceState;
        SurfStateParams.bUseAdvState          = pSurfaceEntry->bAVS;
        SurfStateParams.dwWidth               = pSurfaceEntry->dwWidth;
        SurfStateParams.dwHeight              = pSurfaceEntry->dwHeight;
        SurfStateParams.dwFormat              = pSurfaceEntry->dwFormat;
        SurfStateParams.dwPitch               = pSurfaceEntry->dwPitch;
        SurfStateParams.dwQPitch              = pSurfaceEntry->dwQPitch;
        SurfStateParams.bTiledSurface         = pSurfaceEntry->bTiledSurface;
        SurfStateParams.bTileWalk             = pSurfaceEntry->bTileWalk;
        SurfStateParams.dwCacheabilityControl = pRenderHal->pfnGetSurfaceMemoryObjectControl(pRenderHal, pParams);
        SurfStateParams.RotationMode          = g_cLookup_RotationMode_g12[pRenderHalSurface->Rotation];
        SurfStateParams.TileModeGMM           = pSurface->TileModeGMM;
        SurfStateParams.bGMMTileEnabled       = pSurface->bGMMTileEnabled;

        #if (_DEBUG || _RELEASE_INTERNAL)
            pParams->MemObjCtl                              = SurfStateParams.dwCacheabilityControl;
            pSurface->oldCacheSetting                       = (SurfStateParams.dwCacheabilityControl >> 1) & 0x0000003f;
            if (pParams->isOutput)
            {
                pRenderHal->oldCacheSettingForTargetSurface = pSurface->oldCacheSetting;
            }
        #endif

        if (pSurface->MmcState == MOS_MEMCOMP_RC)
        {
            m_renderHalMMCEnabled    = MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrE2ECompression);
            pRenderHal->isMMCEnabled = m_renderHalMMCEnabled;
        }

        if (IsFormatMMCSupported(pSurface->Format) &&
            m_renderHalMMCEnabled)
        {
            if (pSurface->MmcState == MOS_MEMCOMP_MC ||
                pSurface->MmcState == MOS_MEMCOMP_RC)
            {
                SurfStateParams.MmcState            = pSurface->MmcState;

                if (pSurfaceEntry->YUVPlane == MHW_U_PLANE &&
                   (pSurface->Format        == Format_NV12 ||
                    pSurface->Format        == Format_P010 ||
                    pSurface->Format        == Format_P016))
                {
                    SurfStateParams.dwCompressionFormat = (uint32_t)(0x00000010)
                        | (pSurface->CompressionFormat & 0x0f);
                }
                else if ((pSurface->Format == Format_R8G8UN) &&
                    (pSurface->MmcState == MOS_MEMCOMP_MC))
                {
                    /* it will be an issue if the R8G8UN surface with MC enable
                       is not chroma plane from NV12 surface, so far there is no
                       such case
                    */
                    SurfStateParams.dwCompressionFormat = (uint32_t)(0x00000010)
                        | (pSurface->CompressionFormat & 0x0f);
                }
                else
                {
                    SurfStateParams.dwCompressionFormat = pSurface->CompressionFormat & 0x1f;
                }
            }
            else
            {
                MHW_RENDERHAL_NORMALMESSAGE("Unsupported Compression Mode for Render Engine.");
                SurfStateParams.MmcState            = MOS_MEMCOMP_DISABLED;
                SurfStateParams.dwCompressionFormat = 0;
            }

            if (pParams->isOutput                    &&
                pSurface->MmcState == MOS_MEMCOMP_RC &&
                pSurface->OsResource.bUncompressedWriteNeeded)
            {
                MHW_RENDERHAL_NORMALMESSAGE("force uncompressed write if requested from resources");
                SurfStateParams.MmcState            = MOS_MEMCOMP_MC;
                SurfStateParams.dwCompressionFormat = 0;
            }

            if (!pParams->isOutput                         &&
                pSurface->MmcState != MOS_MEMCOMP_DISABLED &&
                pSurfaceEntry->bVertStride)
            {
                // If input surface is interlaced, then surface should be uncompressed
                // Remove compression setting for such surface
                MHW_RENDERHAL_NORMALMESSAGE("interlaced input for Render Engine.");
                SurfStateParams.MmcState            = MOS_MEMCOMP_DISABLED;
                SurfStateParams.dwCompressionFormat = 0;
            }
        }

        // 2D/3D Surface (non-AVS)
        SurfStateParams.SurfaceType3D             = (pSurface->dwDepth > 1) ?
                                                       GFX3DSTATE_SURFACETYPE_3D :
                                                       GFX3DSTATE_SURFACETYPE_2D;
        SurfStateParams.dwDepth                   = MOS_MAX(1, pSurface->dwDepth);
        SurfStateParams.bVerticalLineStrideOffset = pSurfaceEntry->bVertStrideOffs;
        SurfStateParams.bVerticalLineStride       = pSurfaceEntry->bVertStride;
        SurfStateParams.bHalfPitchChroma          = pSurfaceEntry->bHalfPitchChroma;
        SurfStateParams.bBoardColorOGL            = pParams->bWidthInDword_UV ? false : true;  //sampler surface

        // Setup surface state
        if (pSurfaceEntry->YUVPlane == MHW_U_PLANE ||
            pSurfaceEntry->YUVPlane == MHW_V_PLANE)
        {
            pPlaneOffset = (pSurfaceEntry->YUVPlane == MHW_U_PLANE) ?
                            &pSurface->UPlaneOffset : &pSurface->VPlaneOffset;

            // Get Pixels Per Sample if we use dataport read
            if(pParams->bWidthInDword_UV)
            {
                RenderHal_GetPixelsPerSample(pSurface->Format, &dwPixelsPerSampleUV);
            }
            else
            {
                // If the kernel uses sampler - do not change width (it affects coordinates)
                dwPixelsPerSampleUV = 1;
            }

            if(dwPixelsPerSampleUV == 1)
            {
                SurfStateParams.iXOffset = pPlaneOffset->iXOffset;
            }
            else
            {
                SurfStateParams.iXOffset = pPlaneOffset->iXOffset/sizeof(uint32_t);
            }

            SurfStateParams.iYOffset = pPlaneOffset->iYOffset;
        }
        else // Y plane
        {
            pPlaneOffset = &pSurface->YPlaneOffset;
            SurfStateParams.iXOffset = pPlaneOffset->iXOffset/sizeof(uint32_t);
            SurfStateParams.iYOffset = pPlaneOffset->iYOffset;

            if((pSurfaceEntry->YUVPlane == MHW_Y_PLANE) &&
               (pSurfaceEntry->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8))
            {
                if (pSurface->Format == Format_YV12)
                {
                    SurfStateParams.bSeperateUVPlane = true;
                    SurfStateParams.dwXOffsetForU    = 0;
                    SurfStateParams.dwYOffsetForU    = pSurface->dwHeight * 2 + pSurface->dwHeight / 2;
                    SurfStateParams.dwXOffsetForV    = 0;
                    SurfStateParams.dwYOffsetForV    = pSurface->dwHeight * 2;
                }
                else
                {
                    SurfStateParams.bSeperateUVPlane = false;
                    SurfStateParams.dwXOffsetForU    = 0;
                    SurfStateParams.dwYOffsetForU    = (uint32_t)((pSurface->UPlaneOffset.iSurfaceOffset - pSurface->YPlaneOffset.iSurfaceOffset) / pSurface->dwPitch) + pSurface->UPlaneOffset.iYOffset;
                    SurfStateParams.dwXOffsetForV    = 0;
                    SurfStateParams.dwYOffsetForV    = 0;
                }
            }

            if((pSurfaceEntry->YUVPlane == MHW_Y_PLANE) &&
               (pSurfaceEntry->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_16))
            {
                SurfStateParams.bSeperateUVPlane = false;
                SurfStateParams.dwXOffsetForU    = 0;
                SurfStateParams.dwYOffsetForU    = (uint32_t)((pSurface->UPlaneOffset.iSurfaceOffset - pSurface->YPlaneOffset.iSurfaceOffset) / pSurface->dwPitch) + pSurface->UPlaneOffset.iYOffset;
                SurfStateParams.dwXOffsetForV    = 0;
                SurfStateParams.dwYOffsetForV    = 0;
            }
        }

        // Call MHW to setup the Surface State Heap entry
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->SetSurfaceStateEntry(&SurfStateParams));

        // Setup OS specific states
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSetupSurfaceStatesOs(pRenderHal, pParams, pSurfaceEntry));
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Enables L3 cacheing flag and sets related registers/values
//! \param    PRENDERHAL_INTERFACE    pRenderHal
//!           [in]  Pointer to Hardware Interface
//! \param    pCacheSettings
//!           [in] L3 Cache Configurations
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS XRenderHal_Interface_Xe_Hp_Base::EnableL3Caching(
    PRENDERHAL_INTERFACE         pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings)
{
    VP_FUNC_CALL();

    MOS_STATUS                               eStatus;
    MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12  mHwL3CacheConfig = {};
    PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS     pCacheConfig;
    MhwRenderInterface                       *pMhwRender;
    PRENDERHAL_INTERFACE_LEGACY              pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;

    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    pMhwRender = pRenderHalLegacy->pMhwRenderInterface;
    MHW_RENDERHAL_CHK_NULL(pMhwRender);

    if (nullptr == pCacheSettings)
    {
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->EnableL3Caching(nullptr));
        goto finish;
    }

    // customize the cache config for renderhal and let mhw_render overwrite it
    pCacheConfig = &mHwL3CacheConfig;

    pCacheConfig->dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12HP_RENDERHAL;

    // Override L3 cache configuration
    if (pCacheSettings->bOverride)
    {
        if (pCacheSettings->bCntlRegOverride)
        {
            pCacheConfig->dwCntlReg = pCacheSettings->dwCntlReg;
        }
    }
    MHW_RENDERHAL_CHK_STATUS(pMhwRender->EnableL3Caching(pCacheConfig));

finish:
    return eStatus;
}

//! \brief      Set L3 cache override config parameters
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in,out] pCacheSettings
//!             Pointer to pCacheSettings
//! \param      [in] bEnableSLM
//!             Flag to enable SLM
//! \return     MOS_STATUS
//!             MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS XRenderHal_Interface_Xe_Hp_Base::SetCacheOverrideParams(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
    bool                            bEnableSLM)
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pCacheSettings);
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    pCacheSettings->dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12HP_RENDERHAL;

    pCacheSettings->bCntlRegOverride = true;

finish:
    return eStatus;
}
MOS_STATUS XRenderHal_Interface_Xe_Hp_Base::AllocateScratchSpaceBuffer(
    uint32_t perThreadScratchSpace,
    RENDERHAL_INTERFACE *renderHal)
{
    VP_FUNC_CALL();

    if (m_scratchSpaceResource.iSize > 0)
    {  // Already allocated.
        return MOS_STATUS_SUCCESS;
    }

    const MEDIA_SYSTEM_INFO *gt_sys_info = renderHal->pOsInterface
            ->pfnGetGtSystemInfo(renderHal->pOsInterface);
    uint32_t hw_threads_per_eu = gt_sys_info->ThreadCount/gt_sys_info->EUCount;
    uint32_t scratch_space_entries = gt_sys_info->MaxEuPerSubSlice
            *hw_threads_per_eu*gt_sys_info->MaxSubSlicesSupported;
    uint32_t scratch_space_size
            = scratch_space_entries*perThreadScratchSpace;

    MOS_ALLOC_GFXRES_PARAMS alloc_param;
    MOS_ZeroMemory(&alloc_param, sizeof(alloc_param));
    alloc_param.Type          = MOS_GFXRES_SCRATCH;
    alloc_param.Format        = Format_Buffer;
    alloc_param.dwBytes       = scratch_space_size;
    alloc_param.pSystemMemory = nullptr;
    alloc_param.TileType      = MOS_TILE_LINEAR;
    alloc_param.pBufName      = "ScratchSpaceBuffer";

    return renderHal->pOsInterface->pfnAllocateResource(
        renderHal->pOsInterface, &alloc_param,
        &m_scratchSpaceResource);
}

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
MOS_STATUS XRenderHal_Interface_Xe_Hp_Base::SendComputeWalker(
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
    pGpGpuWalkerParams->IndirectDataStartAddress = pGpGpuWalkerParams->IndirectDataStartAddress + pRenderHalLegacy->pStateHeap->pCurMediaState->dwOffset;

    MHW_RENDERHAL_CHK_STATUS(static_cast<MhwRenderInterfaceXe_Xpm_Base*>(pRenderHalLegacy->pMhwRenderInterface)->AddComputeWalkerCmd(pCmdBuffer,
        pGpGpuWalkerParams,
        &mhwIdEntryParams,
        nullptr,
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
MOS_STATUS XRenderHal_Interface_Xe_Hp_Base::SendTo3DStateBindingTablePoolAlloc(
    PRENDERHAL_INTERFACE        pRenderHal,
    PMOS_COMMAND_BUFFER         pCmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus;
    mhw_render_xe_xpm_base::_3DSTATE_BINDING_TABLE_POOL_ALLOC_CMD cmd;
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;

    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHalLegacy->pMhwRenderInterface);

    MHW_RENDERHAL_CHK_STATUS(static_cast<MhwRenderInterfaceXe_Xpm_Base*>(pRenderHalLegacy->pMhwRenderInterface)->Add3DStateBindingTablePoolAllocCmd(pCmdBuffer, cmd));

finish:
    return eStatus;
}
