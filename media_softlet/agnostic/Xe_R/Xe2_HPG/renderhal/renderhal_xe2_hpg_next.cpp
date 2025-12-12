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
#define ENLARGED_SSH_BINDING_TABLES_XE2_HPG   16

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
    ENLARGED_SSH_BINDING_TABLES_XE2_HPG,    //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES_MAX,      //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT_XE2_HPG, //!< iSurfacesPerBT
    ENLARGE_KERNEL_COUNT_XE2_HPG,          //!< iKernelCount
    ENLARGE_KERNEL_HEAP_XE2_HPG,           //!< iKernelHeapSize
    ENLARGE_CURBE_SIZE_XE2_HPG,            //!< iCurbeSize
    RENDERHAL_MEDIA_IDS                    //!< iMediaIDs
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

    auto &computerModeParams                       = m_renderItf->MHW_GETPAR_F(STATE_COMPUTE_MODE)();
    computerModeParams                             = {};
    computerModeParams.enableLargeGrf              = pRenderHal->largeGrfMode;
    computerModeParams.forceEuThreadSchedulingMode = pRenderHal->euThreadSchedulingMode;
    computerModeParams.enableVariableRegisterSizeAllocationVrt = pRenderHal->enableVariableRegisterSizeAllocationVrt;
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
    pRenderHal->defaultStateHeapSettings       = g_cRenderHal_State_Heap_Settings_xe2_hpg;
    pRenderHal->enlargeStateHeapSettingsForAdv = g_cRenderHal_Enlarge_State_Heap_Settings_Adv_xe2_hpg;
}

//!
//! \brief    Setup Surface State
//! \details  Setup Surface State for Gen11
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Render Hal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pParams
//!           [in]  Pointer to Surface State Params
//! \param    int32_t *piNumEntries
//!           [out] Pointer to Number of Surface State Entries (Num Planes)
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY * ppSurfaceEntries
//!           [out] Array of Surface State Entries
//! \param    PRENDERHAL_OFFSET_OVERRIDE pOffsetOverride
//!           [in] Ignored (not used in Gen11)
//! \return   MOS_STATUS
//!
MOS_STATUS XRenderHal_Interface_Xe2_Hpg_Next::SetupSurfaceState(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams,
    int32_t                        *piNumEntries,
    PRENDERHAL_SURFACE_STATE_ENTRY *ppSurfaceEntries,
    PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride)
{
    VP_FUNC_CALL();

    PRENDERHAL_SURFACE_STATE_ENTRY pSurfaceEntry;
    PMOS_PLANE_OFFSET              pPlaneOffset;
    MHW_SURFACE_STATE_PARAMS       SurfStateParams;
    PMOS_SURFACE                   pSurface;
    int32_t                        i;
    uint32_t                       dwPixelsPerSampleUV;
    uint32_t                       dwSurfaceSize;
    MOS_STATUS                     eStatus = MOS_STATUS_UNKNOWN;

    //-----------------------------------------
    MHW_RENDERHAL_UNUSED(pOffsetOverride);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL_RETURN(pParams);
    MHW_RENDERHAL_CHK_NULL_RETURN(ppSurfaceEntries);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pHwSizes);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal->pMhwStateHeap);

    int32_t index = pRenderHalSurface->Rotation;
    if (!(index >= 0 && index < 8))
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Invalid Rotation");
    }

    dwSurfaceSize = pRenderHal->pHwSizes->dwSizeSurfaceState;

    MOS_ZeroMemory(&SurfStateParams, sizeof(SurfStateParams));

    // Get the Surface State Entries
    MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pfnGetSurfaceStateEntries(
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
        MHW_RENDERHAL_CHK_NULL_RETURN(pSurface);

        // Set the Surface State Offset from base of SSH
        pSurfaceEntry->dwSurfStateOffset = pRenderHal->pStateHeap->iSurfaceStateOffset +  // Offset to Base Of Current Surface State Area
                                           pSurfaceEntry->iSurfStateID * dwSurfaceSize;   // Offset  to Surface State within the area

        // Obtain the Pointer to the Surface state from SSH Buffer
        if (pSurfaceEntry->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_L8_UNORM && !IsL8FormatSupported())
        {
            pSurfaceEntry->dwFormat = MHW_GFX3DSTATE_SURFACEFORMAT_R8_UNORM;
        }
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
        SurfStateParams.RotationMode          = g_cLookup_RotationMode_hpg_base[pRenderHalSurface->Rotation];
        SurfStateParams.TileModeGMM           = pSurface->TileModeGMM;
        SurfStateParams.bGMMTileEnabled       = pSurface->bGMMTileEnabled;

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
                SurfStateParams.MmcState = pSurface->MmcState;

                if (pSurfaceEntry->YUVPlane == MHW_U_PLANE &&
                    (pSurface->Format == Format_NV12 ||
                        pSurface->Format == Format_P010 ||
                        pSurface->Format == Format_P016))
                {
                    SurfStateParams.dwCompressionFormat = (uint32_t)(0x00000010) | (pSurface->CompressionFormat & 0x0f);
                }
                else if ((pSurface->Format == Format_R8G8UN) &&
                         (pSurface->MmcState == MOS_MEMCOMP_MC))
                {
                    /* it will be an issue if the R8G8UN surface with MC enable
                       is not chroma plane from NV12 surface, so far there is no
                       such case
                    */
                    SurfStateParams.dwCompressionFormat = (uint32_t)(0x00000010) | (pSurface->CompressionFormat & 0x0f);
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

            if (pParams->isOutput &&
                pSurface->MmcState == MOS_MEMCOMP_RC &&
                pSurface->OsResource.bUncompressedWriteNeeded)
            {
                MHW_RENDERHAL_NORMALMESSAGE("force uncompressed write if requested from resources");
                SurfStateParams.MmcState            = MOS_MEMCOMP_MC;
                SurfStateParams.dwCompressionFormat = 0;
            }

            if (!pParams->isOutput &&
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
        SurfStateParams.SurfaceType3D             = (pSurface->dwDepth > 1) ? GFX3DSTATE_SURFACETYPE_3D : GFX3DSTATE_SURFACETYPE_2D;
        SurfStateParams.dwDepth                   = MOS_MAX(1, pSurface->dwDepth);
        SurfStateParams.bVerticalLineStrideOffset = pSurfaceEntry->bVertStrideOffs;
        SurfStateParams.bVerticalLineStride       = pSurfaceEntry->bVertStride;
        SurfStateParams.bHalfPitchChroma          = pSurfaceEntry->bHalfPitchChroma;
        SurfStateParams.bBoardColorOGL            = pParams->bWidthInDword_UV ? false : true;  //sampler surface

        // Setup surface state
        if (pSurfaceEntry->YUVPlane == MHW_U_PLANE ||
            pSurfaceEntry->YUVPlane == MHW_V_PLANE)
        {
            pPlaneOffset = (pSurfaceEntry->YUVPlane == MHW_U_PLANE) ? &pSurface->UPlaneOffset : &pSurface->VPlaneOffset;

            // Get Pixels Per Sample if we use dataport read
            if (pParams->bWidthInDword_UV)
            {
                RenderHal_GetPixelsPerSample(pSurface->Format, &dwPixelsPerSampleUV);
            }
            else
            {
                // If the kernel uses sampler - do not change width (it affects coordinates)
                dwPixelsPerSampleUV = 1;
            }

            if (dwPixelsPerSampleUV == 1)
            {
                SurfStateParams.iXOffset = pPlaneOffset->iXOffset;
            }
            else
            {
                SurfStateParams.iXOffset = pPlaneOffset->iXOffset / sizeof(uint32_t);
            }

            SurfStateParams.iYOffset = pPlaneOffset->iYOffset;
        }
        else  // Y plane
        {
            pPlaneOffset             = &pSurface->YPlaneOffset;
            SurfStateParams.iXOffset = pPlaneOffset->iXOffset / sizeof(uint32_t);
            SurfStateParams.iYOffset = pPlaneOffset->iYOffset;

            if (MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrVpFP16))
            {
                if (pSurfaceEntry->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_R16G16B16A16_FLOAT)
                {
                    if (SurfStateParams.iXOffset > 0 && pSurface->MipTailStartLOD < 15)
                    {
                        SurfStateParams.iXOffset = 0;
                        SurfStateParams.MipTailStartLOD = MHW_MIP_TAIL_START_LOD_ENABLED_MASK | pSurface->MipTailStartLOD;
                    }
                }
            }

            if ((pSurfaceEntry->YUVPlane == MHW_Y_PLANE) &&
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

            if ((pSurfaceEntry->YUVPlane == MHW_Y_PLANE) &&
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
        MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pMhwStateHeap->SetSurfaceStateEntry(&SurfStateParams));

        // Setup OS specific states
        MHW_RENDERHAL_CHK_STATUS_RETURN(pRenderHal->pfnSetupSurfaceStatesOs(pRenderHal, pParams, pSurfaceEntry));
    }

    eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}