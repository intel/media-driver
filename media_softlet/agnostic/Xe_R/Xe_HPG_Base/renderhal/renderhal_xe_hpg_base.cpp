/*
* Copyright (c) 2021-2022, Intel Corporation
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

#include "renderhal.h"
#include "renderhal_xe_hpg_base.h"
#include "vp_utils.h"
#include "media_packet.h"
#include "media_feature.h"
//!
//! \brief      GSH settings for Xe_hpg_base
//!
#define RENDERHAL_SAMPLERS_AVS_HPG_BASE          0

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
    RENDERHAL_SSH_BINDING_TABLE_ALIGN          //!< iBTAlignment
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
    MOS_ZeroMemory(&m_scratchSpaceResource, sizeof(m_scratchSpaceResource));
    return;
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
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL(pParams);
    MHW_RENDERHAL_CHK_NULL(ppSurfaceEntries);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
    MHW_RENDERHAL_ASSERT(pRenderHalSurface->Rotation >= 0 && pRenderHalSurface->Rotation < 8);
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
        pSurfaceEntry->dwSurfStateOffset = pRenderHal->pStateHeap->iSurfaceStateOffset +  // Offset to Base Of Current Surface State Area
                                           pSurfaceEntry->iSurfStateID * dwSurfaceSize;   // Offset  to Surface State within the area

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
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwStateHeap->SetSurfaceStateEntry(&SurfStateParams));

        // Setup OS specific states
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pfnSetupSurfaceStatesOs(pRenderHal, pParams, pSurfaceEntry));
    }

    eStatus = MOS_STATUS_SUCCESS;

finish:
    return eStatus;
}

//!
//! \brief    Encode SLM Size for Interface Descriptor
//! \details  Setup SLM size
//! \param    uint32_t SLMSize
//!           [in] SLM size in 1K
//! \return   encoded output
//!
uint32_t XRenderHal_Interface_Xe_Hpg_Base::EncodeSLMSize(uint32_t SLMSize)
{
    uint32_t EncodedValue;
    if (SLMSize <= 2)
    {
        EncodedValue = SLMSize;
    }
    else 
    {
        EncodedValue = 0;
        do
        {
            SLMSize >>= 1;
            EncodedValue++;
        } while (SLMSize);
    }
    return EncodedValue;
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
//! \brief      Setup Chroma direction for Gen11
//! \details    Setup Chroma direction
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to HW Interface
//! \param      PRENDERHAL_SURFACE      pSurface
//!             [in]    Pointer to surface
//! \return     uint8_t
//!
uint8_t XRenderHal_Interface_Xe_Hpg_Base::SetChromaDirection(
    PRENDERHAL_INTERFACE pRenderHal,
    PRENDERHAL_SURFACE   pRenderHalSurface)
{
    uint8_t Direction;
    MHW_RENDERHAL_UNUSED(pRenderHal);
    
    MHW_RENDERHAL_ASSERT(pRenderHal);
    MHW_RENDERHAL_ASSERT(pRenderHalSurface);

    Direction = 0;

    if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_HORZ_CENTER)
    {
        Direction = CHROMA_SITING_UDIRECTION_CENTER;
    }
    else
    {
        Direction = CHROMA_SITING_UDIRECTION_LEFT;
    }

    // Combined U/V direction together in one uint8_t, 1 bit for U direction, 3 bits for V direction.
    Direction = Direction << 3;

    if (pRenderHalSurface->pDeinterlaceParams || pRenderHalSurface->bQueryVariance)
    {
        if ((pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD) ||
            (pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD))
        {
            if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
            {
                Direction |= CHROMA_SITING_VDIRECTION_1_2;
            }
            else if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM)
            {
                Direction |= CHROMA_SITING_VDIRECTION_1;
            }
            else
            {
                Direction |= CHROMA_SITING_VDIRECTION_3_4;
            }
        }
        else if ((pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD) ||
            (pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD))
        {
            if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
            {
                Direction |= CHROMA_SITING_VDIRECTION_0;
            }
            else if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM)
            {
                Direction |= CHROMA_SITING_VDIRECTION_1_2;
            }
            else
            {
                Direction |= CHROMA_SITING_VDIRECTION_1_4;
            }
        }
    }
    else
    {
        if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_TOP)
        {
            Direction |= CHROMA_SITING_VDIRECTION_0;
        }
        else if (pRenderHalSurface->ChromaSiting & MHW_CHROMA_SITING_VERT_BOTTOM)
        {
            Direction |= CHROMA_SITING_VDIRECTION_1;
        }
        else
        {
            Direction |= CHROMA_SITING_VDIRECTION_1_2;
        }
    }

    return Direction;
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
    pRenderHal->StateHeapSettings = g_cRenderHal_State_Heap_Settings_hpg_base;
}

//!
//! \brief    Initialize the default surface type and advanced surface type  per platform
//! \param    PRENDERHAL_INTERFACE    pRenderHal
//!           [out] Pointer to PRENDERHAL_INTERFACE
//! \return   void
//!
void XRenderHal_Interface_Xe_Hpg_Base::InitSurfaceTypes(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set default / advanced surface types
    pRenderHal->SurfaceTypeDefault            = RENDERHAL_SURFACE_TYPE_G10;
    pRenderHal->SurfaceTypeAdvanced           = RENDERHAL_SURFACE_TYPE_ADV_G10;
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

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(m_renderItf);

    if (nullptr == pCacheSettings)
    {
        MHW_RENDERHAL_CHK_STATUS(m_renderItf->EnableL3Caching(nullptr));
        goto finish;
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
    MHW_RENDERHAL_CHK_STATUS(m_renderItf->EnableL3Caching(&cacheConfig));
    
finish:
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

//!
//! \brief    Set Power Option Status
//! \param    [in] pRenderHal
//!           Pointer to Hardware Interface
//! \param    [in,out] pCmdBuffer
//!           Pointer to Command Buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SetPowerOptionStatus(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer)
{
    PMOS_INTERFACE              pOsInterface;
    MOS_STATUS                  eStatus;
    MEDIA_SYSTEM_INFO           *pGtSystemInfo;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);

    eStatus         = MOS_STATUS_SUCCESS;
    pOsInterface    = pRenderHal->pOsInterface;
    pGtSystemInfo   = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pGtSystemInfo);

    // Check if Slice Shutdown can be enabled
    if (pRenderHal->bRequestSingleSlice)
    {
        pCmdBuffer->Attributes.dwNumRequestedEUSlices = 1;
    }
    else if (pRenderHal->bEUSaturationNoSSD)
    {
        pCmdBuffer->Attributes.dwNumRequestedEUSlices = 2;
    }

    if ((pRenderHal->pSkuTable) && (MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrSSEUPowerGating) || MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrSSEUPowerGatingControlByUMD)))
    {
        // VP does not request subslice shutdown according to the array VpHalDefaultSSEUTableGxx
        if (((pRenderHal->PowerOption.nSlice != 0) || (pRenderHal->PowerOption.nSubSlice != 0) || (pRenderHal->PowerOption.nEU != 0)) &&
            ((pGtSystemInfo->SliceCount != 0) && (pGtSystemInfo->SubSliceCount != 0)))
        {
            pCmdBuffer->Attributes.dwNumRequestedEUSlices    = MOS_MIN(pRenderHal->PowerOption.nSlice, pGtSystemInfo->SliceCount);
            pCmdBuffer->Attributes.dwNumRequestedSubSlices   = MOS_MIN(pRenderHal->PowerOption.nSubSlice, (pGtSystemInfo->SubSliceCount / pGtSystemInfo->SliceCount));
            pCmdBuffer->Attributes.dwNumRequestedEUs         = MOS_MIN(pRenderHal->PowerOption.nEU, (pGtSystemInfo->EUCount / pGtSystemInfo->SubSliceCount));
            pCmdBuffer->Attributes.bValidPowerGatingRequest  = true;
            pCmdBuffer->Attributes.bUmdSSEUEnable            = true;
        }
    }

finish:
    return eStatus;
}

//!
//! \brief    Set Composite Prolog CMD
//! \param    [in] pRenderHal
//!           Pointer to Hardware Interface
//! \param    [in,out] pCmdBuffer
//!           Pointer to Command Buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SetCompositePrologCmd(
    PRENDERHAL_INTERFACE pRenderHal, 
    PMOS_COMMAND_BUFFER  pCmdBuffer)
{
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    uint64_t                              auxTableBaseAddr = 0;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(m_miItf);

    auxTableBaseAddr = pRenderHal->pOsInterface->pfnGetAuxTableBaseAddr(pRenderHal->pOsInterface);

    if (auxTableBaseAddr)
    {
        auto& parImm = m_miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
        parImm = {};
        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_RCS_AUX_TABLE_BASE_LOW);
        parImm.dwData = (auxTableBaseAddr & 0xffffffff);
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_RCS_AUX_TABLE_BASE_HIGH);
        parImm.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_CCS0_AUX_TABLE_BASE_LOW);
        parImm.dwData = (auxTableBaseAddr & 0xffffffff);
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);

        parImm.dwRegister = m_miItf->GetMmioInterfaces(mhw::mi::MHW_MMIO_CCS0_AUX_TABLE_BASE_HIGH);
        parImm.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        m_miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(pCmdBuffer);
    }

finish:
    return eStatus;
}

MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::IsRenderHalMMCEnabled(
    PRENDERHAL_INTERFACE         pRenderHal)
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
    UserFeatureData.bData = true;  // turn on MMC for xe_hpg
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

MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SendComputeWalker(
    PRENDERHAL_INTERFACE     pRenderHal,
    PMOS_COMMAND_BUFFER      pCmdBuffer,
    PMHW_GPGPU_WALKER_PARAMS pGpGpuWalkerParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                eStatus = MOS_STATUS_SUCCESS;
    MHW_ID_ENTRY_PARAMS       mhwIdEntryParams;
    PRENDERHAL_KRN_ALLOCATION pKernelEntry;
    PRENDERHAL_MEDIA_STATE    pCurMediaState;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pGpGpuWalkerParams);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pKernelAllocation);

    MOS_ZeroMemory(&mhwIdEntryParams, sizeof(mhwIdEntryParams));

    pKernelEntry   = &pRenderHal->pStateHeap->pKernelAllocation[pRenderHal->iKernelAllocationID];
    pCurMediaState = pRenderHal->pStateHeap->pCurMediaState;

    MHW_RENDERHAL_CHK_NULL(pKernelEntry);
    MHW_RENDERHAL_CHK_NULL(pCurMediaState);

    mhwIdEntryParams.dwKernelOffset  = pKernelEntry->dwOffset;
    mhwIdEntryParams.dwSamplerCount  = pKernelEntry->Params.Sampler_Count;
    mhwIdEntryParams.dwSamplerOffset = pCurMediaState->dwOffset +
                                       pRenderHal->pStateHeap->dwOffsetSampler +
                                       pGpGpuWalkerParams->InterfaceDescriptorOffset * pRenderHal->pStateHeap->dwSizeSampler;
    mhwIdEntryParams.dwBindingTableOffset          = pGpGpuWalkerParams->BindingTableID * pRenderHal->pStateHeap->iBindingTableSize;
    mhwIdEntryParams.dwSharedLocalMemorySize       = pGpGpuWalkerParams->SLMSize;
    mhwIdEntryParams.dwNumberofThreadsInGPGPUGroup = pGpGpuWalkerParams->ThreadWidth * pGpGpuWalkerParams->ThreadHeight;
    //This only a WA to disable EU fusion for multi-layer blending cases or single layer do colorfill and rotation together.
    //Need remove it after kernel or compiler fix it.
    mhwIdEntryParams.bBarrierEnable              = pRenderHal->eufusionBypass ? 1 : 0;
    pGpGpuWalkerParams->IndirectDataStartAddress = pGpGpuWalkerParams->IndirectDataStartAddress + pRenderHal->pStateHeap->pCurMediaState->dwOffset;

    MHW_RENDERHAL_CHK_NULL(m_renderItf);
    m_gpgpuWalkerParams         = pGpGpuWalkerParams;
    m_interfaceDescriptorParams = &mhwIdEntryParams;
    SETPAR_AND_ADDCMD(COMPUTE_WALKER, m_renderItf, pCmdBuffer);

finish:
    return eStatus;
}

//!
//! \brief    Check if Override is needed or not
//! \param    [in] pRenderHal
//!           Pointer to Hardware Interface
//! \param    [in,out] pCmdBuffer
//!           Pointer to Command Buffer
//! \param    [in] pGenericPrologParam
//!           Pointer to MHW generic prolog parameters
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::IsOvrdNeeded(
    PRENDERHAL_INTERFACE              pRenderHal,
    PMOS_COMMAND_BUFFER               pCmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS  pGenericPrologParams)
{
    PMOS_INTERFACE                        pOsInterface;
    MOS_STATUS                            eStatus;
    PMOS_CMD_BUF_ATTRI_VE                 pAttriVe;
    PRENDERHAL_GENERIC_PROLOG_PARAMS_HPG_BASE pGenericPrologParamsHPG_BASE;
    uint8_t                               i;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);

    eStatus                 = MOS_STATUS_SUCCESS;
    pOsInterface            = pRenderHal->pOsInterface;
    pAttriVe               = (PMOS_CMD_BUF_ATTRI_VE)(pCmdBuffer->Attributes.pAttriVe);
    pGenericPrologParamsHPG_BASE = dynamic_cast<PRENDERHAL_GENERIC_PROLOG_PARAMS_HPG_BASE>(pGenericPrologParams);

    // Split Frame
    if (pOsInterface->VEEnable)
    {
#if !EMUL
        if (pGenericPrologParamsHPG_BASE)
#else
        if (pGenericPrologParamsHPG_BASE && pAttriVe != nullptr)
#endif
        {
            // Split Frame
            if (pGenericPrologParamsHPG_BASE->VEngineHintParams.BatchBufferCount > 1)
            {
                pAttriVe->bUseVirtualEngineHint = true;
                pAttriVe->VEngineHintParams = pGenericPrologParamsHPG_BASE->VEngineHintParams;
            }
        }

#if (_DEBUG || _RELEASE_INTERNAL)
#if !EMUL
        if (pOsInterface->bEnableDbgOvrdInVE)
#else
        if (pOsInterface->bEnableDbgOvrdInVE && pAttriVe != nullptr)
#endif
        {
            if (pOsInterface->bVeboxScalabilityMode)
            {
                pAttriVe->VEngineHintParams.DebugOverride = true;
#if !EMUL
                if (pGenericPrologParamsHPG_BASE)
#else
                if (pGenericPrologParamsHPG_BASE && pAttriVe != nullptr)
#endif
                {
                    pAttriVe->VEngineHintParams.BatchBufferCount = pGenericPrologParamsHPG_BASE->VEngineHintParams.BatchBufferCount;
                    for (i = 0; i < pGenericPrologParamsHPG_BASE->VEngineHintParams.BatchBufferCount; i++)
                    {
                        pAttriVe->VEngineHintParams.EngineInstance[i] = i;
                    }
                }
            }
            else if (pOsInterface->eForceVebox)
            {
                pAttriVe->VEngineHintParams.DebugOverride = true;
                pAttriVe->VEngineHintParams.BatchBufferCount = 1;
                pAttriVe->VEngineHintParams.EngineInstance[0] = pOsInterface->eForceVebox - 1;
            }
        }
#endif
    }

finish:
    return eStatus;
};

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

    MHW_RENDERHAL_CHK_NULL(pCacheSettings);
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    pCacheSettings->dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_XE_HPG_BASE_RENDERHAL;

    pCacheSettings->bCntlRegOverride = true;

finish:
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

//! \brief      Get Surface Compression support caps
//! \param      [in] format
//!             surface format
//! \return     bool
//!             true or false
bool XRenderHal_Interface_Xe_Hpg_Base::IsFormatMMCSupported(MOS_FORMAT format)
{
    // Check if Sample Format is supported
    if ((format != Format_YUY2)             &&
        (format != Format_Y410)             &&
        (format != Format_Y216)             &&
        (format != Format_Y210)             &&
        (format != Format_Y416)             &&
        (format != Format_P010)             &&
        (format != Format_P016)             &&
        (format != Format_AYUV)             &&
        (format != Format_NV21)             &&
        (format != Format_NV12)             &&
        (format != Format_UYVY)             &&
        (format != Format_YUYV)             &&
        (format != Format_A8B8G8R8)         &&
        (format != Format_X8B8G8R8)         &&
        (format != Format_A8R8G8B8)         &&
        (format != Format_X8R8G8B8)         &&
        (format != Format_B10G10R10A2)      &&
        (format != Format_R10G10B10A2)      &&
        (format != Format_A16R16G16B16F)    &&
        (format != Format_IMC3)             &&
        (format != Format_444P)             &&
        (format != Format_422H)             &&
        (format != Format_422V)             &&
        (format != Format_411P)             &&
        (format != Format_411R)             &&
        (format != Format_444P)             &&
        (format != Format_RGBP)             &&
        (format != Format_BGRP)             &&
        (format != Format_400P)             &&
        (format != Format_420O)             &&
        (format != Format_R8UN)             &&
        (format != Format_A8)               &&
        (format != Format_R8G8UN))
    {
        MHW_RENDERHAL_NORMALMESSAGE("Unsupported Format '0x%08x' for Render MMC.", format);
        return false;
    }

    return true;
}

MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::AllocateScratchSpaceBuffer(
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
    uint32_t hw_threads_per_eu     = gt_sys_info->ThreadCount / gt_sys_info->EUCount;
    uint32_t scratch_space_entries = gt_sys_info->MaxEuPerSubSlice * hw_threads_per_eu * gt_sys_info->MaxSubSlicesSupported;
    uint32_t scratch_space_size    = scratch_space_entries * perThreadScratchSpace;

    MOS_ALLOC_GFXRES_PARAMS alloc_param;
    MOS_ZeroMemory(&alloc_param, sizeof(alloc_param));
    alloc_param.Type          = MOS_GFXRES_SCRATCH;
    alloc_param.Format        = Format_Buffer;
    alloc_param.dwBytes       = scratch_space_size;
    alloc_param.pSystemMemory = nullptr;
    alloc_param.TileType      = MOS_TILE_LINEAR;
    alloc_param.pBufName      = "ScratchSpaceBuffer";

    return renderHal->pOsInterface->pfnAllocateResource(
        renderHal->pOsInterface, &alloc_param, &m_scratchSpaceResource);
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

MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::FreeScratchSpaceBuffer(
    RENDERHAL_INTERFACE *renderHal)
{
    MOS_GFXRES_FREE_FLAGS resFreeFlags = {0};

    resFreeFlags.AssumeNotInUse = 1;

    if (m_scratchSpaceResource.iSize <= 0)
    {
        return MOS_STATUS_SUCCESS;  // Scratch space is not allocated. No need to free resources.
    }

    renderHal->pOsInterface
            ->pfnFreeResourceWithFlag(renderHal->pOsInterface,
                                      &m_scratchSpaceResource,
                                      resFreeFlags.Value);
    renderHal->pOsInterface
            ->pfnResetResourceAllocationIndex(renderHal->pOsInterface,
                                              &m_scratchSpaceResource);
    return MOS_STATUS_SUCCESS;
}

//! \brief    Send To 3DState Binding Table Pool Alloc
//! \details    Send To 3DState Binding Table Pool Alloc
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!            [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!            [in] Pointer to Command Buffer
//! \return   MOS_STATUS
MOS_STATUS XRenderHal_Interface_Xe_Hpg_Base::SendTo3DStateBindingTablePoolAlloc(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(m_renderItf);
    SETPAR_AND_ADDCMD(_3DSTATE_BINDING_TABLE_POOL_ALLOC, m_renderItf, pCmdBuffer);

finish:
    return eStatus;
}

MHW_SETPAR_DECL_SRC(STATE_BASE_ADDRESS, XRenderHal_Interface_Xe_Hpg_Base)
{
    MHW_STATE_BASE_ADDR_PARAMS* pStateBaseParams = nullptr;
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    pStateBaseParams = &m_renderHal->StateBaseAddressParams;
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateBaseParams);

    params.presGeneralState = pStateBaseParams->presGeneralState;
    params.dwGeneralStateSize = pStateBaseParams->dwGeneralStateSize;
    params.presDynamicState = pStateBaseParams->presDynamicState;
    params.dwDynamicStateSize = pStateBaseParams->dwDynamicStateSize;
    params.bDynamicStateRenderTarget = pStateBaseParams->bDynamicStateRenderTarget;
    params.presIndirectObjectBuffer = pStateBaseParams->presIndirectObjectBuffer;
    params.dwIndirectObjectBufferSize = pStateBaseParams->dwIndirectObjectBufferSize;
    params.presInstructionBuffer = pStateBaseParams->presInstructionBuffer;
    params.dwInstructionBufferSize = pStateBaseParams->dwInstructionBufferSize;
    params.mocs4InstructionCache = pStateBaseParams->mocs4InstructionCache;
    params.mocs4GeneralState = pStateBaseParams->mocs4GeneralState;
    params.mocs4DynamicState = pStateBaseParams->mocs4DynamicState;
    params.mocs4SurfaceState = pStateBaseParams->mocs4SurfaceState;
    params.mocs4IndirectObjectBuffer = pStateBaseParams->mocs4IndirectObjectBuffer;
    params.mocs4StatelessDataport = pStateBaseParams->mocs4StatelessDataport;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(_3DSTATE_CHROMA_KEY, XRenderHal_Interface_Xe_Hpg_Base)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PMHW_CHROMAKEY_PARAMS       pChromaKeyParams = nullptr;
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    pChromaKeyParams = m_renderHal->ChromaKey;
    MHW_RENDERHAL_CHK_NULL_RETURN(pChromaKeyParams);

    params.dwIndex = pChromaKeyParams->dwIndex;
    params.dwLow = pChromaKeyParams->dwLow;
    params.dwHigh = pChromaKeyParams->dwHigh;

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(STATE_SIP, XRenderHal_Interface_Xe_Hpg_Base)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MHW_SIP_STATE_PARAMS        SipStateParams = {};
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    SipStateParams = m_renderHal->SipStateParams;

    params.dwSipBase = SipStateParams.dwSipBase;

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

MHW_SETPAR_DECL_SRC(COMPUTE_WALKER, XRenderHal_Interface_Xe_Hpg_Base)
{
    MHW_RENDERHAL_CHK_NULL_RETURN(m_gpgpuWalkerParams);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_interfaceDescriptorParams);

    params.IndirectDataLength = m_gpgpuWalkerParams->IndirectDataLength;
    params.IndirectDataStartAddress = m_gpgpuWalkerParams->IndirectDataStartAddress;
    params.ThreadWidth = m_gpgpuWalkerParams->ThreadWidth;
    params.ThreadHeight = m_gpgpuWalkerParams->ThreadHeight;
    params.ThreadDepth = m_gpgpuWalkerParams->ThreadDepth;

    params.GroupWidth = m_gpgpuWalkerParams->GroupWidth;
    params.GroupHeight = m_gpgpuWalkerParams->GroupHeight;
    params.GroupDepth = m_gpgpuWalkerParams->GroupDepth;
    params.GroupStartingX = m_gpgpuWalkerParams->GroupStartingX;
    params.GroupStartingY = m_gpgpuWalkerParams->GroupStartingY;
    params.GroupStartingZ = m_gpgpuWalkerParams->GroupStartingZ;

    params.dwKernelOffset = m_interfaceDescriptorParams->dwKernelOffset;
    params.dwSamplerCount = m_interfaceDescriptorParams->dwSamplerCount;
    params.dwSamplerOffset = m_interfaceDescriptorParams->dwSamplerOffset;
    params.dwBindingTableOffset = m_interfaceDescriptorParams->dwBindingTableOffset;;
    params.bBarrierEnable = m_interfaceDescriptorParams->bBarrierEnable;
    params.dwNumberofThreadsInGPGPUGroup = m_interfaceDescriptorParams->dwNumberofThreadsInGPGPUGroup;
    params.dwSharedLocalMemorySize = m_interfaceDescriptorParams->dwSharedLocalMemorySize;
    params.IndirectDataStartAddress = m_gpgpuWalkerParams->IndirectDataStartAddress;

    if (m_gpgpuWalkerParams->ThreadDepth == 0)
    {
        params.ThreadDepth = 1;
    }
    if (m_gpgpuWalkerParams->GroupDepth == 0)
    {
        params.GroupDepth = 1;
    }

    return MOS_STATUS_SUCCESS;
}
