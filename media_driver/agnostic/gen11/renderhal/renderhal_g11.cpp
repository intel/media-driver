/*
* Copyright (c) 2017-2020, Intel Corporation
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
//! \file       renderhal_g11.cpp
//! \brief      implementation of Gen11 hardware functions
//! \details    Render functions
//!

#include "renderhal_legacy.h"
#include "renderhal_g11.h"

//!
//! \brief      GSH settings for G11
//!
extern const RENDERHAL_STATE_HEAP_SETTINGS g_cRenderHal_State_Heap_Settings_g11 =
{
    // Global GSH Allocation parameters
    RENDERHAL_SYNC_SIZE,                       //!< iSyncSize

    // Media State Allocation parameters
    RENDERHAL_MEDIA_STATES,                    //!< iMediaStateHeaps - Set by Initialize
    RENDERHAL_MEDIA_IDS,                       //!< iMediaIDs
    RENDERHAL_CURBE_SIZE,                      //!< iCurbeSize
    RENDERHAL_SAMPLERS,                        //!< iSamplers
    RENDERHAL_SAMPLERS_AVS_G11,                //!< iSamplersAVS
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


const uint32_t g_cLookup_RotationMode_g11[8] = 
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

#define RENDERHAL_NS_PER_TICK_RENDER_G11        (83.333)                                  // Assume it same as SKL, 83.333 nano seconds per tick in render engine

//!
//! DSH State Heap settings for G11
//!
const RENDERHAL_DYN_HEAP_SETTINGS g_cRenderHal_DSH_Settings_g11 =
{
    0x0080000,  // dwDshInitialSize    = 512MB
    0x0080000,  // dwDshSizeIncrement  = 512kB
    0x8000000,  // dwDshMaximumSize    = 128MB (all heaps)
    0x0100000,  // dwIshInitialSize    = 1M
    0x0040000,  // dwIshSizeIncrement  = 256kB
    0x0400000,  // dwIshMaximumSize    = 4MB
    16,         // iMinMediaStates
    256,        // iMaxMediaStates
    16,         // iMinKernels
    2048        // iMaxKernels
};

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
MOS_STATUS XRenderHal_Interface_g11::SetupSurfaceState (
    PRENDERHAL_INTERFACE             pRenderHal,
    PRENDERHAL_SURFACE               pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS  pParams,
    int32_t                          *piNumEntries,
    PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
    PRENDERHAL_OFFSET_OVERRIDE       pOffsetOverride)
{
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
    MHW_RENDERHAL_ASSERT(pRenderHalSurface->Rotation >= MHW_ROTATION_IDENTITY &&
                         pRenderHalSurface->Rotation <= MHW_ROTATE_90_MIRROR_HORIZONTAL);
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
        SurfStateParams.bCompressionEnabled   = pSurface->bIsCompressed;
        SurfStateParams.bCompressionMode      = (pSurface->CompressionMode == MOS_MMC_VERTICAL) ? 1 : 0;
        SurfStateParams.RotationMode          = g_cLookup_RotationMode_g11[pRenderHalSurface->Rotation];

        if (pSurfaceEntry->bAVS)
        {
            SurfStateParams.bHalfPitchChroma        = pSurfaceEntry->bHalfPitchChroma;
            SurfStateParams.bInterleaveChroma       = pSurfaceEntry->bInterleaveChroma;
            SurfStateParams.UVPixelOffsetUDirection = pSurfaceEntry->DirectionU;
            SurfStateParams.UVPixelOffsetVDirection = pSurfaceEntry->DirectionV;

            // On SNB+, when VDI Walker is enabled, Input surface width should be 16 pixel aligned
            if (pParams->bWidth16Align)
            {
                SurfStateParams.dwWidth = MOS_ALIGN_CEIL(pSurfaceEntry->dwWidth, 16);
            }

            if (pSurfaceEntry->YUVPlane == MHW_U_PLANE)         // AVS U plane
            {
                // Lockoffset is the offset from base address of Y plane to the origin of U/V plane.
                // So, We can get XOffsetforU by Lockoffset % pSurface->dwPitch, and get YOffsetForU by Lockoffset / pSurface->dwPitch
                SurfStateParams.dwXOffsetForU = (uint32_t)pSurface->UPlaneOffset.iLockSurfaceOffset % pSurface->dwPitch;
                SurfStateParams.dwYOffsetForU = (uint32_t)pSurface->UPlaneOffset.iLockSurfaceOffset / pSurface->dwPitch;
                SurfStateParams.dwXOffsetForV = 0;
                SurfStateParams.dwYOffsetForV = 0;
                SurfStateParams.iXOffset      = pSurface->UPlaneOffset.iXOffset;
                SurfStateParams.iYOffset      = pSurface->UPlaneOffset.iYOffset;
            }
            else if (pSurfaceEntry->YUVPlane == MHW_V_PLANE)    // AVS V plane
            {
                SurfStateParams.dwXOffsetForU = 0;
                SurfStateParams.dwYOffsetForU = 0;
                SurfStateParams.dwXOffsetForV = (uint32_t)pSurface->VPlaneOffset.iLockSurfaceOffset % pSurface->dwPitch;
                SurfStateParams.dwYOffsetForV = (uint32_t)pSurface->VPlaneOffset.iLockSurfaceOffset / pSurface->dwPitch;
                SurfStateParams.iXOffset      = pSurface->VPlaneOffset.iXOffset;
                SurfStateParams.iYOffset      = pSurface->VPlaneOffset.iYOffset;
            }
            else // AVS/DNDI Y plane
            {
                SurfStateParams.dwXOffsetForU = pSurfaceEntry->wUXOffset;
                SurfStateParams.dwYOffsetForU = pSurfaceEntry->wUYOffset;
                SurfStateParams.dwXOffsetForV = pSurfaceEntry->wVXOffset;
                SurfStateParams.dwYOffsetForV = pSurfaceEntry->wVYOffset;
                SurfStateParams.iXOffset      = 0;
                SurfStateParams.iYOffset      = pSurface->YPlaneOffset.iYOffset;
            }
            if (pRenderHalSurface->bInterlacedScaling)
            {
                SurfStateParams.bVerticalLineStrideOffset = pSurfaceEntry->bVertStrideOffs;
                SurfStateParams.bVerticalLineStride       = pSurfaceEntry->bVertStride;
            }
        }
        else // 2D/3D Surface (non-AVS)
        {
            SurfStateParams.SurfaceType3D             = (pSurface->dwDepth > 1) ?
                                                           GFX3DSTATE_SURFACETYPE_3D :
                                                           GFX3DSTATE_SURFACETYPE_2D;
            SurfStateParams.dwDepth                   = MOS_MAX(1, pSurface->dwDepth);
            SurfStateParams.bVerticalLineStrideOffset = pSurfaceEntry->bVertStrideOffs;
            SurfStateParams.bVerticalLineStride       = pSurfaceEntry->bVertStride;
            SurfStateParams.bHalfPitchChroma          = pSurfaceEntry->bHalfPitchChroma;

            // Setup surface g9 surface state
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
uint32_t XRenderHal_Interface_g11::EncodeSLMSize(uint32_t SLMSize)
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
void XRenderHal_Interface_g11::ConvertToNanoSeconds(
    PRENDERHAL_INTERFACE                 pRenderHal,
    uint64_t                            iTicks,
    uint64_t                            *piNs)
{
    //-----------------------------
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(piNs);
    //-----------------------------
    *piNs = (uint64_t)(iTicks * RENDERHAL_NS_PER_TICK_RENDER_G11);
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
uint8_t XRenderHal_Interface_g11::SetChromaDirection(
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
void XRenderHal_Interface_g11::InitStateHeapSettings(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set State Heap settings for g11
    pRenderHal->StateHeapSettings = g_cRenderHal_State_Heap_Settings_g11;
}

//!
//! \brief    Initialize the default surface type and advanced surface type  per platform
//! \param    PRENDERHAL_INTERFACE    pRenderHal
//!           [out] Pointer to PRENDERHAL_INTERFACE
//! \return   void
//!
void XRenderHal_Interface_g11::InitSurfaceTypes(
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
MOS_STATUS XRenderHal_Interface_g11::EnableL3Caching(
    PRENDERHAL_INTERFACE         pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings)
{
    MOS_STATUS                           eStatus;
    MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G11 mHwL3CacheConfig = {};
    PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS pCacheConfig;
    MhwRenderInterface                   *pMhwRender;
    PRENDERHAL_INTERFACE_LEGACY          pRenderHalLegacy = (PRENDERHAL_INTERFACE_LEGACY)pRenderHal;
    
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
    
    pCacheConfig->dwCntlReg  = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G11_RENDERHAL;

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

//!
//! \brief    Get offset and/or pointer to sampler state
//! \details  Get offset and/or pointer to sampler state in General State Heap
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface
//! \param    int32_t iMediaID
//!           [in] Media ID associated with sampler
//! \param    int32_t iSamplerID
//!           [in] Sampler ID
//! \param    uint32_t *pdwSamplerOffset
//!           [out] optional; offset of sampler state from GSH base
//! \param    void  **ppSampler
//!           [out] optional; pointer to sampler state in GSH
//! \return   MOS_STATUS
//!
MOS_STATUS XRenderHal_Interface_g11::GetSamplerOffsetAndPtr_DSH(
    PRENDERHAL_INTERFACE     pRenderHal,
    int32_t                  iMediaID,
    int32_t                  iSamplerID,
    PMHW_SAMPLER_STATE_PARAM pSamplerParams,
    uint32_t                 *pdwSamplerOffset,
    void                    **ppSampler)
{
    PRENDERHAL_STATE_HEAP       pStateHeap;
    PRENDERHAL_DYNAMIC_STATE    pDynamicState;
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    uint32_t                    dwSamplerIndirect;
    uint32_t                    dwOffset;
    MHW_SAMPLER_TYPE            SamplerType;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap->pCurMediaState);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);

    pStateHeap    = pRenderHal->pStateHeap;
    pDynamicState = ((PRENDERHAL_MEDIA_STATE_LEGACY)pStateHeap->pCurMediaState)->pDynamicState;

    MHW_RENDERHAL_CHK_NULL(pDynamicState);

    MHW_RENDERHAL_ASSERT(iMediaID   < pDynamicState->MediaID.iCount);

    dwOffset    = iMediaID * pDynamicState->dwSizeSamplers;                    // Go to Media ID sampler offset

    SamplerType = (pSamplerParams) ? pSamplerParams->SamplerType : MHW_SAMPLER_TYPE_3D;

    switch (SamplerType)
    {
        case MHW_SAMPLER_TYPE_AVS:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerAVS.iCount);
            dwOffset += pDynamicState->SamplerAVS.dwOffset +                    // Go to AVS sampler area
                        iSamplerID * MHW_SAMPLER_STATE_AVS_INC_G11;              // 16: size of one element, 128 elements for SKL
            break;

        case MHW_SAMPLER_TYPE_CONV:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerConv.iCount);
            dwOffset = pDynamicState->SamplerConv.dwOffset;                     // Goto Conv sampler base
            if ( pSamplerParams->Convolve.ui8ConvolveType == 0 && pSamplerParams->Convolve.skl_mode )
            {   // 2D convolve
                dwOffset += iSamplerID * MHW_SAMPLER_STATE_CONV_INC_G11;         // 16: size of one element, 128 elements for SKL
            }
            else if ( pSamplerParams->Convolve.ui8ConvolveType == 1 )
            {   // 1D convolve
                dwOffset += iSamplerID * MHW_SAMPLER_STATE_CONV_1D_INC;      // 16: size of one element, 8 elements for SKL
            }
            else
            {   // 1P convolve (same as gen8) and 2D convolve BDW mode
                dwOffset += iSamplerID * MHW_SAMPLER_STATE_CONV_INC_LEGACY;  // 16: size of one element, 32: 32 entry
            }
            break;

        case MHW_SAMPLER_TYPE_MISC:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerMisc.iCount);
            dwOffset += pDynamicState->Sampler3D.dwOffset          +             // Goto sampler base
                        iSamplerID * MHW_SAMPLER_STATE_VA_INC;                   // 16: size of one element, 2: 2 entries
            break;

        case MHW_SAMPLER_TYPE_3D:
        case MHW_SAMPLER_TYPE_VME:
        default:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->Sampler3D.iCount);
            dwSamplerIndirect = dwOffset;
            dwOffset += pDynamicState->Sampler3D.dwOffset          +             // Go 3D Sampler base
                        iSamplerID * pRenderHal->pHwSizes->dwSizeSamplerState;   // Goto to "samplerID" sampler state

            if (pSamplerParams)
            {
                dwSamplerIndirect += pDynamicState->SamplerInd.dwOffset +                              // offset to indirect sampler area
                                     iSamplerID * pRenderHal->pHwSizes->dwSizeSamplerIndirectState;   // Goto to "samplerID" indirect state
                pSamplerParams->Unorm.IndirectStateOffset = dwSamplerIndirect;
            }

            break;
    }

    if (pdwSamplerOffset)
    {
        *pdwSamplerOffset = dwOffset;
    }

finish:
    return eStatus;
}

//!
//! \brief      Initialize the DSH Settings
//! \details    Initialize the structure DynamicHeapSettings in pRenderHal
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to HW interface
//! \return     void
//!
void XRenderHal_Interface_g11::InitDynamicHeapSettings(
    PRENDERHAL_INTERFACE  pRenderHal)
{
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = static_cast<PRENDERHAL_INTERFACE_LEGACY>(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHalLegacy);
    // Additional Dynamic State Heap settings for g11
    pRenderHalLegacy->DynamicHeapSettings           = g_cRenderHal_DSH_Settings_g11;
}

//!
//! \brief    Set Power Option Status
//! \param    [in] pRenderHal
//!           Pointer to Hardware Interface
//! \param    [in/out] pCmdBuffer
//!           Pointer to Command Buffer
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS XRenderHal_Interface_g11::SetPowerOptionStatus(
    PRENDERHAL_INTERFACE         pRenderHal,
    PMOS_COMMAND_BUFFER          pCmdBuffer)
{
    // deprecated after enabled per-context SSEU. 
    return MOS_STATUS_SUCCESS;
}

//!
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
MOS_STATUS XRenderHal_Interface_g11::SetCacheOverrideParams(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
    bool                            bEnableSLM)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pCacheSettings);

    pCacheSettings->dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G11_RENDERHAL;
    pCacheSettings->bCntlRegOverride = true;

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
MOS_STATUS XRenderHal_Interface_g11::IsOvrdNeeded(
    PRENDERHAL_INTERFACE              pRenderHal,
    PMOS_COMMAND_BUFFER               pCmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS  pGenericPrologParams)
{
    PMOS_INTERFACE                        pOsInterface;
    MOS_STATUS                            eStatus;
    PMOS_CMD_BUF_ATTRI_VE                pAttriVe;
    PRENDERHAL_GENERIC_PROLOG_PARAMS_G11  pGenericPrologParamsG11;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);

    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = pRenderHal->pOsInterface;
    pAttriVe    = (PMOS_CMD_BUF_ATTRI_VE)(pCmdBuffer->Attributes.pAttriVe);
    pGenericPrologParamsG11 = dynamic_cast<PRENDERHAL_GENERIC_PROLOG_PARAMS_G11>(pGenericPrologParams);

    if (pAttriVe)
    {
        if (pGenericPrologParamsG11)
        {
            // Split Frame
            if (pGenericPrologParamsG11->VEngineHintParams.BatchBufferCount == 2 && pOsInterface->VEEnable)
            {
                pAttriVe->bUseVirtualEngineHint = true;
                pAttriVe->VEngineHintParams = pGenericPrologParamsG11->VEngineHintParams;
            }
        }

#if (_DEBUG || _RELEASE_INTERNAL)
        if (pOsInterface->bEnableDbgOvrdInVE)
        {
            if (pOsInterface->bVeboxScalabilityMode)
            {
                pAttriVe->VEngineHintParams.DebugOverride     = true;
                pAttriVe->VEngineHintParams.BatchBufferCount  = 2;
                pAttriVe->VEngineHintParams.EngineInstance[0] = 0;
                pAttriVe->VEngineHintParams.EngineInstance[1] = 1;
            }
            else if (pOsInterface->eForceVebox)
            {
                pAttriVe->VEngineHintParams.DebugOverride     = true;
                pAttriVe->VEngineHintParams.BatchBufferCount  = 1;
                pAttriVe->VEngineHintParams.EngineInstance[0] = pOsInterface->eForceVebox - 1;
            }
        }
#endif
    }

finish:
    return eStatus;
};

//! \brief      Get the size of Render Surface State Command
//! \return     size_t
//!             the size of render surface state command
size_t XRenderHal_Interface_g11::GetSurfaceStateCmdSize()
{
    return MOS_ALIGN_CEIL( MOS_MAX(mhw_state_heap_g11_X::RENDER_SURFACE_STATE_CMD::byteSize,
                   mhw_state_heap_g11_X::MEDIA_SURFACE_STATE_CMD::byteSize), MHW_SURFACE_STATE_ALIGN);
}

