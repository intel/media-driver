/*
* Copyright (c) 2021-2023, Intel Corporation
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
//! \file       renderhal_xe_hpg_base.cpp
//! \brief      implementation of Gen12 hardware functions
//! \details    Render functions
//!

#include "renderhal_xe_hpg_base.h"
#include "vp_utils.h"

//!
//! \brief      GSH settings for Xe_hpg_base
//!
#define RENDERHAL_SAMPLERS_AVS_HPG_BASE        0
#define ENLARGE_KERNEL_COUNT_HPG_BASE          RENDERHAL_KERNEL_COUNT * 3
#define ENLARGE_KERNEL_HEAP_HPG_BASE           RENDERHAL_KERNEL_HEAP * 3
#define ENLARGE_CURBE_SIZE_HPG_BASE            RENDERHAL_CURBE_SIZE * 16

extern const RENDERHAL_STATE_HEAP_SETTINGS g_cRenderHal_State_Heap_Settings_hpg_base =
{
    // Global GSH Allocation parameters
    RENDERHAL_SYNC_SIZE,                       //!< iSyncSize

    // Media State Allocation parameters
    RENDERHAL_MEDIA_STATES,                    //!< iMediaStateHeaps - Set by Initialize
    RENDERHAL_MEDIA_IDS,                       //!< iMediaIDs
    RENDERHAL_CURBE_SIZE,                      //!< iCurbeSize
    RENDERHAL_SAMPLERS,                        //!< iSamplers
    RENDERHAL_SAMPLERS_AVS_HPG_BASE,           //!< iSamplersAVS
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

extern const RENDERHAL_ENLARGE_PARAMS g_cRenderHal_Enlarge_State_Heap_Settings_Adv_hpg_base =
{
    RENDERHAL_SSH_BINDING_TABLES_MAX,       //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES_MAX,       //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT,          //!< iSurfacesPerBT
    ENLARGE_KERNEL_COUNT_HPG_BASE,          //!< iKernelCount
    ENLARGE_KERNEL_HEAP_HPG_BASE,           //!< iKernelHeapSize
    ENLARGE_CURBE_SIZE_HPG_BASE             //!< iCurbeSize
};

const uint32_t g_cLookup_RotationMode_hpg_base[8] = 
{
    ROTATION_IDENTITY,  // 0 - MHW_ROTATION_IDENTITY
    ROTATION_90,        // 1 - MHW_ROTATION_90
    ROTATION_180,       // 2 - MHW_ROTATION_180
    ROTATION_270,       // 3 - MHW_ROTATION_270
    ROTATION_IDENTITY,  // 4 - MHW_MIRROR_HORIZONTAL
    ROTATION_180,       // 5 - MHW_MIRROR_VERTICAL
    ROTATION_270,       // 6 - MHW_ROTATE_90_MIRROR_VERTICAL
    ROTATION_90         // 7 - MHW_ROTATE_90_MIRROR_HORIZONTAL
};

#define RENDERHAL_NS_PER_TICK_RENDER_HPG_BASE        (83.333)                                  // Assume it same as SKL, 83.333 nano seconds per tick in render engine

XRenderHal_Interface_Xe_Hpg_Base::XRenderHal_Interface_Xe_Hpg_Base()
{
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
MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SetupSurfaceState(
    PRENDERHAL_INTERFACE             pRenderHal,
    PRENDERHAL_SURFACE               pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS  pParams,
    int32_t                          *piNumEntries,
    PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
    PRENDERHAL_OFFSET_OVERRIDE       pOffsetOverride)
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

    int32_t index            = pRenderHalSurface->Rotation;
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
                SurfStateParams.MmcState = MOS_MEMCOMP_MC;
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

//!
//! \brief    Convert To Nano Seconds
//! \details  Convert to Nano Seconds
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    uint64_t iTicks
//!           [in] Ticks
//! \param    uint64_t *piNs
//!           [in] Nano Seconds
//! \return   void
//!
void XRenderHal_Interface_Xe_Hpg_Base::ConvertToNanoSeconds(
    PRENDERHAL_INTERFACE                 pRenderHal,
    uint64_t                            iTicks,
    uint64_t                            *piNs)
{
    //-----------------------------
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(piNs);
    //-----------------------------
    *piNs = (uint64_t)(iTicks * RENDERHAL_NS_PER_TICK_RENDER_HPG_BASE);
}

//!
//! \brief    Initialize the State Heap Settings per platform
//! \param    PRENDERHAL_STATE_HEAP_SETTINGS pSettings
//!           [out] Pointer to PRENDERHAL_STATE_HEAP_SETTINGSStructure
//! \return   void
//!
void XRenderHal_Interface_Xe_Hpg_Base::InitStateHeapSettings(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set State Heap settings for hpg_base
    pRenderHal->StateHeapSettings              = g_cRenderHal_State_Heap_Settings_hpg_base;
    pRenderHal->enlargeStateHeapSettingsForAdv = g_cRenderHal_Enlarge_State_Heap_Settings_Adv_hpg_base;
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
MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::EnableL3Caching(
    PRENDERHAL_INTERFACE         pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings)
{
    VP_FUNC_CALL();

    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    mhw::render::MHW_RENDER_ENGINE_L3_CACHE_SETTINGS cacheConfig = {};

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    if (nullptr == pCacheSettings)
    {
        MHW_RENDERHAL_CHK_STATUS_RETURN(m_renderItf->EnableL3Caching(nullptr));
        return eStatus;
    }
    cacheConfig.dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_XE_HPG_BASE_RENDERHAL;
    // Override L3 cache configuration
    if (pCacheSettings->bOverride)
    {
        if (pCacheSettings->bCntlRegOverride)
        {
            cacheConfig.dwCntlReg = pCacheSettings->dwCntlReg;
        }
    }
    MHW_RENDERHAL_CHK_STATUS_RETURN(m_renderItf->EnableL3Caching(&cacheConfig));
    
    return eStatus;
}

void XRenderHal_Interface_Xe_Hpg_Base::SetFusedEUDispatch(bool enable)
{
    m_vfeStateParams.bFusedEuDispatch = enable? true : false;
}

MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SetNumOfWalkers(uint32_t numOfWalkers)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    // value: [0,1] - One or two active walkers per context.
    if (numOfWalkers > 2)
    {
        m_vfeStateParams.numOfWalkers = 1;
    }
    else if (numOfWalkers > 0)
    {
        m_vfeStateParams.numOfWalkers = numOfWalkers - 1;
    }
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
MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SetCacheOverrideParams(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
    bool                            bEnableSLM)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pCacheSettings);
    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);

    pCacheSettings->dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_XE_HPG_BASE_RENDERHAL;

    pCacheSettings->bCntlRegOverride = true;

    return eStatus;
}

//! \brief      Get the size of Render Surface State Command
//! \return     size_t
//!             the size of render surface state command
size_t XRenderHal_Interface_Xe_Hpg_Base::GetSurfaceStateCmdSize()
{
    return MOS_ALIGN_CEIL( MOS_MAX(mhw_state_heap_xe_hpg::RENDER_SURFACE_STATE_CMD::byteSize,
                   mhw_state_heap_xe_hpg::MEDIA_SURFACE_STATE_CMD::byteSize), MHW_SURFACE_STATE_ALIGN);
}

static const uint32_t FIXED_SCRATCH_SPACE_BUFFER_INDEX = 6;

MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SetScratchSpaceBufferState(
    RENDERHAL_INTERFACE *renderHal,
    uint32_t indexOfBindingTable)
{
    if (m_scratchSpaceResource.iSize <= 0)
    {
        return MOS_STATUS_SUCCESS;  // Scratch space is not allocated. No need to set states.
    }

    RENDERHAL_SURFACE renderhal_surface;
    MOS_ZeroMemory(&renderhal_surface, sizeof(renderhal_surface));
    renderhal_surface.OsSurface.OsResource = m_scratchSpaceResource;
    renderhal_surface.OsSurface.dwWidth = m_scratchSpaceResource.iSize;
    renderhal_surface.OsSurface.dwHeight = 1;
    renderhal_surface.OsSurface.Format = Format_RAW;
    renderhal_surface.OsSurface.Type = MOS_GFXRES_SCRATCH;
    renderhal_surface.rcSrc.right = m_scratchSpaceResource.iSize;;
    renderhal_surface.rcSrc.bottom = 1;
    renderhal_surface.rcDst = renderhal_surface.rcSrc;

    MOS_STATUS result = renderHal->pOsInterface->pfnRegisterResource(
        renderHal->pOsInterface, &(renderhal_surface.OsSurface.OsResource),
        true, true);
    if (MOS_STATUS_SUCCESS != result)
    {
        return result;
    }

    RENDERHAL_SURFACE_STATE_PARAMS renderhal_surface_state_param;
    MOS_ZeroMemory(&renderhal_surface_state_param,
                   sizeof(renderhal_surface_state_param));
    renderhal_surface_state_param.isOutput = 1;
    renderhal_surface_state_param.MemObjCtl = 2;

    RENDERHAL_SURFACE_STATE_ENTRY *renderhal_surface_state_entry = nullptr;
    renderHal->pfnSetupBufferSurfaceState(renderHal, &renderhal_surface,
                                          &renderhal_surface_state_param,
                                          &renderhal_surface_state_entry);
    m_vfeStateParams.scratchStateOffset
            = renderhal_surface_state_entry->dwSurfStateOffset;

    renderHal->pfnBindSurfaceState(renderHal,
                                   indexOfBindingTable,
                                   FIXED_SCRATCH_SPACE_BUFFER_INDEX,
                                   renderhal_surface_state_entry);
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(CFE_STATE, XRenderHal_Interface_Xe_Hpg_Base)
{
    MHW_VFE_PARAMS* pVfeStateParams     = nullptr;
    MHW_VFE_PARAMS_XE_HPG* paramsNext   = nullptr;
    pVfeStateParams                     = nullptr;

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal->pHwCaps);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal->pRenderHalPltInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal->pRenderHalPltInterface->GetVfeStateParameters());

    pVfeStateParams     = m_renderHal->pRenderHalPltInterface->GetVfeStateParameters();
    params.pKernelState = pVfeStateParams->pKernelState;

    if (pVfeStateParams->pKernelState)
    {
        params.dwMaximumNumberofThreads = (pVfeStateParams->dwMaximumNumberofThreads) ? pVfeStateParams->dwMaximumNumberofThreads - 1 : pVfeStateParams->pKernelState->KernelParams.iThreadCount - 1;
    }
    else
    {
        params.dwMaximumNumberofThreads = (pVfeStateParams->dwMaximumNumberofThreads) ? pVfeStateParams->dwMaximumNumberofThreads - 1 : m_renderHal->pHwCaps->dwMaxThreads - 1;
    }

    paramsNext = dynamic_cast<MHW_VFE_PARAMS_XE_HPG*>(pVfeStateParams);
    if (paramsNext != nullptr)
    {
        params.ScratchSpaceBuffer = paramsNext->scratchStateOffset >> 6;
        params.FusedEuDispatch = paramsNext->bFusedEuDispatch ? false : true;  // disabled if DW3.FusedEuDispath = 1
        params.NumberOfWalkers = paramsNext->numOfWalkers;
        params.SingleSliceDispatchCcsMode = paramsNext->enableSingleSliceDispatchCcsMode;
    }

    return MOS_STATUS_SUCCESS;
}