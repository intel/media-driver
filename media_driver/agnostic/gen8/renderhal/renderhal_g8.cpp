/*
* Copyright (c) 2011-2020, Intel Corporation
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
//! \file       renderhal_g8.cpp
//! \brief      implementation of Gen8 hardware functions
//! \details    render functions
//!
#include "mos_os.h"
#include "renderhal_legacy.h"
#include "renderhal_g8.h"

//!
//! \brief State Heap Settings for G8
//!
const RENDERHAL_STATE_HEAP_SETTINGS g_cRenderHal_State_Heap_Settings_g8 =
{
    // Global GSH Allocation parameters
    RENDERHAL_SYNC_SIZE,                        //!< iSyncSize

    // Media State Allocation parameters
    RENDERHAL_MEDIA_STATES,                     //!< iMediaStateHeaps - Set by Initialize
    RENDERHAL_MEDIA_IDS,                        //!< iMediaIDs
    RENDERHAL_CURBE_SIZE,                       //!< iCurbeSize
    RENDERHAL_SAMPLERS,                         //!< iSamplers
    RENDERHAL_SAMPLERS_AVS_G8,                  //!< iSamplersAVS
    RENDERHAL_SAMPLERS_VA,                      //!< iSamplersVA
    RENDERHAL_KERNEL_COUNT,                     //!< iKernelCount
    RENDERHAL_KERNEL_HEAP,                      //!< iKernelHeapSize
    RENDERHAL_KERNEL_BLOCK_SIZE,                //!< iKernelBlockSize

    // Media VFE/ID configuration, limits
    0,                                          //!< iPerThreadScratchSize
    RENDERHAL_MAX_SIP_SIZE,                     //!< iSipSize

    // Surface State Heap Settings
    RENDERHAL_SSH_INSTANCES,                    //!< iSurfaceStateHeaps
    RENDERHAL_SSH_BINDING_TABLES,               //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES,               //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT,              //!< iSurfacesPerBT
    RENDERHAL_SSH_BINDING_TABLE_ALIGN           //!< iBTAlignment
};

#define RENDERHAL_NS_PER_TICK_RENDER_G8        80                                  // Prior to Skylake, 80 nano seconds per tick in render engine

//!
//! DSH State Heap settings for G8
//!
const RENDERHAL_DYN_HEAP_SETTINGS g_cRenderHal_DSH_Settings_g8 =
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
//! \details  Setup Surface States for Gen75/Gen8
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to Hardware Interface Structure
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in] Pointer to Render Hal Surface
//! \param    PRENDERHAL_SURFACE_STATE_PARAMS pParams
//!           [in] Pointer to Surface State Params
//! \param    int32_t *piNumEntries
//!           [out] Pointer to Number of Surface State Entries (Num Planes)
//! \param    PRENDERHAL_SURFACE_STATE_ENTRY * ppSurfaceEntries
//!           [out] Array of Surface State Entries
//! \param    PRENDERHAL_OFFSET_OVERRIDE pOffsetOverride
//!           [in] If not nullptr, provides adjustments to Y, UV plane offsets,
//!           used for kernel WA in a few cases. nullptr is the most common usage.
//! \return   MOS_STATUS
//!
MOS_STATUS XRenderHal_Interface_g8::SetupSurfaceState(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_SURFACE              pRenderHalSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS pParams,
    int32_t                         *piNumEntries,
    PRENDERHAL_SURFACE_STATE_ENTRY  *ppSurfaceEntries,
    PRENDERHAL_OFFSET_OVERRIDE      pOffsetOverride)
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
    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pRenderHalSurface);
    MHW_RENDERHAL_CHK_NULL(pParams);
    MHW_RENDERHAL_CHK_NULL(ppSurfaceEntries);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pStateHeap);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pHwSizes);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwStateHeap);
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
        pSurfaceEntry->dwSurfStateOffset = pRenderHal->pStateHeap->iSurfaceStateOffset +               // Offset to Base Of Current Surface State Area
                                           pSurfaceEntry->iSurfStateID * dwSurfaceSize;                // Offset  to Surface State within the area

        // Obtain the Pointer to the Surface state from SSH Buffer
        SurfStateParams.pSurfaceState         = pSurfaceEntry->pSurfaceState;
        SurfStateParams.bUseAdvState          = pSurfaceEntry->bAVS;
        SurfStateParams.dwWidth               = (pParams->bWAUseSrcWidth)  ? pRenderHalSurface->rcSrc.right  : pSurfaceEntry->dwWidth;
        SurfStateParams.dwHeight              = (pParams->bWAUseSrcHeight) ? pRenderHalSurface->rcSrc.bottom : pSurfaceEntry->dwHeight;
        SurfStateParams.dwFormat              = pSurfaceEntry->dwFormat;
        SurfStateParams.dwPitch               = pSurfaceEntry->dwPitch;
        SurfStateParams.dwQPitch              = pSurfaceEntry->dwQPitch;
        SurfStateParams.bTiledSurface         = pSurfaceEntry->bTiledSurface;
        SurfStateParams.bTileWalk             = pSurfaceEntry->bTileWalk;
        SurfStateParams.dwCacheabilityControl = pRenderHal->pfnGetSurfaceMemoryObjectControl(pRenderHal, pParams);

        if (pSurfaceEntry->bAVS)
        {
            SurfStateParams.bHalfPitchChroma        = pSurfaceEntry->bHalfPitchChroma;
            SurfStateParams.bInterleaveChroma       = pSurfaceEntry->bInterleaveChroma;
            SurfStateParams.UVPixelOffsetVDirection = pSurfaceEntry->DirectionV;
            SurfStateParams.AddressControl          = pSurfaceEntry->AddressControl;

            // On SNB+, when VDI Walker is enabled, Input surface width should be 16 pixel aligned
            if (pParams->bWidth16Align)
            {
                SurfStateParams.dwWidth = MOS_ALIGN_CEIL(pSurfaceEntry->dwWidth, 16);
            }

            if (pSurfaceEntry->YUVPlane == MHW_U_PLANE)         // AVS U plane
            {
                SurfStateParams.dwXOffsetForU = (uint32_t)pSurface->UPlaneOffset.iXOffset;
                SurfStateParams.dwYOffsetForU = (uint32_t)pSurface->UPlaneOffset.iYOffset;
                SurfStateParams.dwXOffsetForV = 0;
                SurfStateParams.dwYOffsetForV = 0;
            }
            else if (pSurfaceEntry->YUVPlane == MHW_V_PLANE)    // AVS V plane
            {
                SurfStateParams.dwXOffsetForU = (uint32_t)pSurface->VPlaneOffset.iXOffset;
                SurfStateParams.dwYOffsetForU = (uint32_t)pSurface->VPlaneOffset.iYOffset;
                SurfStateParams.dwXOffsetForV = 0;
                SurfStateParams.dwYOffsetForV = 0;
            }
            else                                                // AVS/DNDI Y plane
            {
                SurfStateParams.dwXOffsetForU = pSurfaceEntry->wUXOffset;
                SurfStateParams.dwYOffsetForU = pSurfaceEntry->wUYOffset;
                SurfStateParams.dwXOffsetForV = pSurfaceEntry->wVXOffset;
                SurfStateParams.dwYOffsetForV = pSurfaceEntry->wVYOffset;
            }
            if (pRenderHalSurface->bInterlacedScaling)
            {
                SurfStateParams.bVerticalLineStrideOffset = pSurfaceEntry->bVertStrideOffs;
                SurfStateParams.bVerticalLineStride       = pSurfaceEntry->bVertStride;
            }
        }
        else // 2D/3D Surface (non-AVS)
        {
            SurfStateParams.SurfaceType3D   =
                    (pSurface->dwDepth > 1) ? GFX3DSTATE_SURFACETYPE_3D :
                                              GFX3DSTATE_SURFACETYPE_2D;
            SurfStateParams.dwDepth                   = MOS_MAX(1, pSurface->dwDepth);
            SurfStateParams.bVerticalLineStrideOffset = pSurfaceEntry->bVertStrideOffs;
            SurfStateParams.bVerticalLineStride       = pSurfaceEntry->bVertStride;
            SurfStateParams.bSurfaceArraySpacing      = true;

            // Setup surface g7 surface state
            if (pSurfaceEntry->YUVPlane == MHW_U_PLANE ||
                pSurfaceEntry->YUVPlane == MHW_V_PLANE)
            {
                pPlaneOffset = (pSurfaceEntry->YUVPlane == MHW_U_PLANE) ?
                                &pSurface->UPlaneOffset : &pSurface->VPlaneOffset;

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

                if (pOffsetOverride)
                {
                    pPlaneOffset->iSurfaceOffset += pOffsetOverride->iUVOffsetAdjust;
                    SurfStateParams.iXOffset     = (dwPixelsPerSampleUV == 1) ?
                                                     pPlaneOffset->iXOffset :    //        is it correct? No override if PixelsPerSamplerUV == 1??
                                                     pOffsetOverride->iUVOffsetX;
                    SurfStateParams.iYOffset     = pOffsetOverride->iUVOffsetY;
                }
                else
                {
                    if (dwPixelsPerSampleUV == 1)
                    {
                        SurfStateParams.iXOffset = pPlaneOffset->iXOffset;
                    }
                    else
                    {
                        SurfStateParams.iXOffset = pPlaneOffset->iXOffset/sizeof(uint32_t);
                    }

                    SurfStateParams.iYOffset     = pPlaneOffset->iYOffset;
                }
            }
            else // Y plane
            {
                pPlaneOffset = &pSurface->YPlaneOffset;

                if (pOffsetOverride)
                {
                    pSurface->dwOffset      += pOffsetOverride->iYOffsetAdjust;
                    SurfStateParams.iXOffset = pOffsetOverride->iYOffsetX;
                    SurfStateParams.iYOffset = pOffsetOverride->iYOffsetY;
                }
                else
                {
                    SurfStateParams.iXOffset = pPlaneOffset->iXOffset/sizeof(uint32_t);
                    SurfStateParams.iYOffset = pPlaneOffset->iYOffset;
                }

                if (pSurfaceEntry->YUVPlane == MHW_Y_PLANE &&
                    pSurfaceEntry->dwFormat == MHW_GFX3DSTATE_SURFACEFORMAT_PLANAR_420_8)
                {
                    SurfStateParams.dwXOffsetForU = 0;
                    SurfStateParams.dwYOffsetForU = pSurface->dwHeight;
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
//! \brief      Checks how per thread scratch space size bits in VFE state are interpreted by HW
//! \details    For BDW GT1/2/3 A0 steppings, per thread scratch space size in VFE state
//!             is 11 bits indicating [2k bytes, 2 Mbytes]: 0=2k, 1=4k, 2=8k ... 10=2M
//!             BDW+ excluding A0 step is 12 bits indicating [1k bytes, 2 Mbytes]: 0=1k, 1=2k, 2=4k, 3=8k ... 11=2M
//! \param      PRENDERHAL_INTERFACE pRenderHal
//!             [in]    Pointer to RenderHal interface
//! \return     true if BDW A0 stepping, false otherwise
//!
bool XRenderHal_Interface_g8::PerThreadScratchSpaceStart2K(
    PRENDERHAL_INTERFACE pRenderHal)
{
    if (pRenderHal == nullptr)
    {
        MHW_RENDERHAL_ASSERTMESSAGE("Null pointer detected.");
        return false;
    }

    // true for BDW GT1/2/3 A0 stepping
    if (pRenderHal->Platform.usRevId == 0)
        return true;
    else
        return false;
}

//!
//! \brief    Encode SLM Size for Interface Descriptor
//! \details  Setup SLM size
//! \param    uint32_t SLMSize
//!           [in] SLM size in 1K
//! \return   encoded output
//!
uint32_t XRenderHal_Interface_g8::EncodeSLMSize(uint32_t SLMSize)
{
    return (MOS_ALIGN_CEIL(SLMSize, 4) >> MHW_SLM_SHIFT);
}

//!
//! \brief    Set Chroma Direction
//! \details  Setup Chroma Direction for G8
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in]  Pointer to Hardware Interface
//! \param    PRENDERHAL_SURFACE pRenderHalSurface
//!           [in]  Pointer to Render Hal Surface
//! \return   uint8_t
//!
uint8_t XRenderHal_Interface_g8::SetChromaDirection(
    PRENDERHAL_INTERFACE pRenderHal,
    PRENDERHAL_SURFACE   pRenderHalSurface)
{
    uint8_t Direction;
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_ASSERT(pRenderHal);
    MHW_RENDERHAL_ASSERT(pRenderHalSurface);

    Direction = MEDIASTATE_VDIRECTION_FULL_FRAME;
    if (pRenderHalSurface->pDeinterlaceParams || pRenderHalSurface->bQueryVariance)
    {
        if ((pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_BOTTOM_FIELD) ||
            (pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_BOTTOM_FIELD))
        {
            Direction    = MEDIASTATE_VDIRECTION_BOTTOM_FIELD;
        }
        else if ((pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_EVEN_FIRST_TOP_FIELD) ||
                 (pRenderHalSurface->SampleType == RENDERHAL_SAMPLE_INTERLEAVED_ODD_FIRST_TOP_FIELD))
        {
            Direction    = MEDIASTATE_VDIRECTION_TOP_FIELD;
        }
    }

    return Direction;
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
void XRenderHal_Interface_g8::ConvertToNanoSeconds(
    PRENDERHAL_INTERFACE                pRenderHal,
    uint64_t                            iTicks,
    uint64_t                            *piNs)
{
    //-----------------------------
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(piNs);
    //-----------------------------

    *piNs = iTicks * RENDERHAL_NS_PER_TICK_RENDER_G8;
}

//!
//! \brief    Initialize the State Heap Settings per platform
//! \param    PRENDERHAL_STATE_HEAP_SETTINGS pSettings
//!           [out] Pointer to PRENDERHAL_STATE_HEAP_SETTINGSStructure
//! \return   void
//!
void XRenderHal_Interface_g8::InitStateHeapSettings(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set State Heap settings for g8
    pRenderHal->StateHeapSettings = g_cRenderHal_State_Heap_Settings_g8;
}

//!
//! \brief    Initialize the default surface type and advanced surface type  per platform
//! \param    PRENDERHAL_INTERFACE    pRenderHal
//!           [out] Pointer to PRENDERHAL_INTERFACE
//! \return   void
//!
void XRenderHal_Interface_g8::InitSurfaceTypes(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set default / advanced surface types
    pRenderHal->SurfaceTypeDefault            = RENDERHAL_SURFACE_TYPE_G8;
    pRenderHal->SurfaceTypeAdvanced           = RENDERHAL_SURFACE_TYPE_ADV_G8;
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
MOS_STATUS XRenderHal_Interface_g8::EnableL3Caching(
    PRENDERHAL_INTERFACE         pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings)
{
    MOS_STATUS                           eStatus;
    PLATFORM                             Platform;
    MHW_RENDER_ENGINE_L3_CACHE_SETTINGS  mHwL3CacheConfig = {};
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

    pRenderHalLegacy->pOsInterface->pfnGetPlatform(pRenderHalLegacy->pOsInterface, &Platform);

    pCacheConfig->dwSqcReg1  = L3_CACHE_SQC1_REG_VALUE_G8;

    pCacheConfig->dwCntlReg = GetL3CacheCntlRegWithSLM();

    // Override L3 cache configuration
    if (pCacheSettings->bOverride)
    {
        if (pCacheSettings->bSqcReg1Override)
        {
            pCacheConfig->dwSqcReg1 = pCacheSettings->dwSqcReg1;
        }

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
MOS_STATUS XRenderHal_Interface_g8::GetSamplerOffsetAndPtr_DSH(
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

    dwOffset    = iMediaID * pDynamicState->dwSizeSamplers;                     // Go to Media ID sampler offset

    SamplerType = (pSamplerParams) ? pSamplerParams->SamplerType : MHW_SAMPLER_TYPE_3D;

    switch (SamplerType)
    {
        case MHW_SAMPLER_TYPE_AVS:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerAVS.iCount);
            dwOffset += pDynamicState->SamplerAVS.dwOffset        +             // Go to AVS sampler area
                        iSamplerID * MHW_SAMPLER_STATE_AVS_INC_LEGACY;  // 16: size of one element, 32: 32 entries
            break;

        case MHW_SAMPLER_TYPE_CONV:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerConv.iCount);
            dwOffset += pDynamicState->SamplerConv.dwOffset        +             // Goto Conv sampler base
                        iSamplerID * MHW_SAMPLER_STATE_CONV_INC_LEGACY;  // 16: size of one element, 32: 32 entries
            break;

        case MHW_SAMPLER_TYPE_MISC:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerMisc.iCount);
            dwOffset += pDynamicState->SamplerMisc.dwOffset        +             // Goto sampler base
                        iSamplerID * MHW_SAMPLER_STATE_VA_INC;                   // 16: size of one element, 2: 2 entries
            break;

        case MHW_SAMPLER_TYPE_3D:
        default:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->Sampler3D.iCount);
            dwSamplerIndirect = dwOffset;
            dwOffset += pDynamicState->Sampler3D.dwOffset          +             // Go 3D Sampler base
                        iSamplerID * pRenderHal->pHwSizes->dwSizeSamplerState;   // Go to "samplerID" sampler state

            if (pSamplerParams)
            {
                dwSamplerIndirect += pDynamicState->SamplerInd.dwOffset +                             // offset to indirect sampler area
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
void XRenderHal_Interface_g8::InitDynamicHeapSettings(
    PRENDERHAL_INTERFACE  pRenderHal)
{
    PRENDERHAL_INTERFACE_LEGACY pRenderHalLegacy = static_cast<PRENDERHAL_INTERFACE_LEGACY>(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHalLegacy);
    // Additional Dynamic State Heap settings for g8
    pRenderHalLegacy->DynamicHeapSettings           = g_cRenderHal_DSH_Settings_g8;
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
MOS_STATUS XRenderHal_Interface_g8::SetPowerOptionStatus(
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

    if ((pRenderHal->pSkuTable) && MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrSSEUPowerGating))
    {
        // VP does not request subslice shutdown according to the array VpHalDefaultSSEUTableGxx
        if (((pRenderHal->PowerOption.nSlice != 0) || (pRenderHal->PowerOption.nSubSlice != 0) || (pRenderHal->PowerOption.nEU != 0)) &&
            ((pGtSystemInfo->SliceCount != 0) && (pGtSystemInfo->SubSliceCount != 0)))
        {
            pCmdBuffer->Attributes.dwNumRequestedEUSlices    = MOS_MIN(pRenderHal->PowerOption.nSlice, pGtSystemInfo->SliceCount);
            pCmdBuffer->Attributes.dwNumRequestedSubSlices   = MOS_MIN(pRenderHal->PowerOption.nSubSlice, (pGtSystemInfo->SubSliceCount / pGtSystemInfo->SliceCount));
            pCmdBuffer->Attributes.dwNumRequestedEUs         = MOS_MIN(pRenderHal->PowerOption.nEU, (pGtSystemInfo->EUCount / pGtSystemInfo->SubSliceCount));
            pCmdBuffer->Attributes.bValidPowerGatingRequest  = true;

            if (pOsInterface->pfnSetSliceCount)
            {
                uint32_t sliceCount = pCmdBuffer->Attributes.dwNumRequestedEUSlices;
                pOsInterface->pfnSetSliceCount(pOsInterface, &sliceCount);
            }
        }
    }

finish:
    return eStatus;
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
MOS_STATUS XRenderHal_Interface_g8::SetCacheOverrideParams(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
    bool                            bEnableSLM)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCacheSettings);

    if (bEnableSLM)
    {
        pCacheSettings->dwCntlReg = RENDERHAL_L3_CACHE_CNTL_REG_SLM_ENABLE_G8;
    }
    else
    {
        pCacheSettings->dwCntlReg = RENDERHAL_L3_CACHE_CNTL_REG_SLM_DISABLE_ALL_L3_512K_G8;
    }
    pCacheSettings->bCntlRegOverride = true;

finish:
    return eStatus;
}

//! \brief      Get the size of Render Surface State Command
//! \return     size_t
//!             the size of render surface state command
size_t XRenderHal_Interface_g8::GetSurfaceStateCmdSize()
{
    return MOS_ALIGN_CEIL( MOS_MAX(mhw_state_heap_g8_X::RENDER_SURFACE_STATE_CMD::byteSize,
                   mhw_state_heap_g8_X::MEDIA_SURFACE_STATE_CMD::byteSize), MHW_SURFACE_STATE_ALIGN);
}

