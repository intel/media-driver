/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file       renderhal_g12.cpp
//! \brief      implementation of Gen11 hardware functions
//! \details    Render functions
//!

#include "renderhal.h"
#include "renderhal_g12.h"
#include "mhw_mi_g12_X.h"

//!
//! \brief      GSH settings for G12
//!
#define RENDERHAL_SAMPLERS_AVS_G12          6

extern const RENDERHAL_STATE_HEAP_SETTINGS g_cRenderHal_State_Heap_Settings_g12 =
{
    // Global GSH Allocation parameters
    RENDERHAL_SYNC_SIZE,                       //!< iSyncSize

    // Media State Allocation parameters
    RENDERHAL_MEDIA_STATES,                    //!< iMediaStateHeaps - Set by Initialize
    RENDERHAL_MEDIA_IDS,                       //!< iMediaIDs
    RENDERHAL_CURBE_SIZE,                      //!< iCurbeSize
    RENDERHAL_SAMPLERS,                        //!< iSamplers
    RENDERHAL_SAMPLERS_AVS_G12,                //!< iSamplersAVS
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

const uint32_t g_cLookup_RotationMode_g12[8] = 
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

#define RENDERHAL_NS_PER_TICK_RENDER_G12        (83.333)                                  // Assume it same as SKL, 83.333 nano seconds per tick in render engine

//!
//! DSH State Heap settings for G12
//!
const RENDERHAL_DYN_HEAP_SETTINGS g_cRenderHal_DSH_Settings_g12 =
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

XRenderHal_Interface_g12::XRenderHal_Interface_g12()
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
MOS_STATUS XRenderHal_Interface_g12::SetupSurfaceState (
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

        if (IsFormatMMCSupported(pSurface->Format) &&
            m_renderHalMMCEnabled)
        {
            // Set surface compression states
            if (pSurface->MmcState == MOS_MEMCOMP_RC && pParams->bRenderTarget)
            {
                // bCompressionEnabled/bCompressionMode is deprecated on Gen12+, use MmcState instead.
                // RC compression mode is not supported on render output surface on tgllp.
                SurfStateParams.MmcState            = MOS_MEMCOMP_DISABLED;
                SurfStateParams.dwCompressionFormat = 0;
            }
            else if (pSurface->MmcState == MOS_MEMCOMP_MC ||
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
        }

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
            SurfStateParams.bBoardColorOGL            = pParams->bWidthInDword_UV ? false : true;  //sampler surface

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
uint32_t XRenderHal_Interface_g12::EncodeSLMSize(uint32_t SLMSize)
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
void XRenderHal_Interface_g12::ConvertToNanoSeconds(
    PRENDERHAL_INTERFACE                 pRenderHal,
    uint64_t                            iTicks,
    uint64_t                            *piNs)
{
    //-----------------------------
    MHW_RENDERHAL_UNUSED(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(piNs);
    //-----------------------------
    *piNs = (uint64_t)(iTicks * RENDERHAL_NS_PER_TICK_RENDER_G12);
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
uint8_t XRenderHal_Interface_g12::SetChromaDirection(
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
void XRenderHal_Interface_g12::InitStateHeapSettings(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set State Heap settings for g12
    pRenderHal->StateHeapSettings = g_cRenderHal_State_Heap_Settings_g12;
}

//!
//! \brief    Initialize the default surface type and advanced surface type  per platform
//! \param    PRENDERHAL_INTERFACE    pRenderHal
//!           [out] Pointer to PRENDERHAL_INTERFACE
//! \return   void
//!
void XRenderHal_Interface_g12::InitSurfaceTypes(
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
MOS_STATUS XRenderHal_Interface_g12::EnableL3Caching(
    PRENDERHAL_INTERFACE         pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS pCacheSettings)
{
    MOS_STATUS                           eStatus;
    MHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G12  mHwL3CacheConfig = {};
    PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS pCacheConfig;
    MhwRenderInterface                   *pMhwRender;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    pMhwRender = pRenderHal->pMhwRenderInterface;
    MHW_RENDERHAL_CHK_NULL(pMhwRender);

    if (nullptr == pCacheSettings)
    {
        MHW_RENDERHAL_CHK_STATUS(pMhwRender->EnableL3Caching(nullptr));
        goto finish;
    }

    // customize the cache config for renderhal and let mhw_render overwrite it
    pCacheConfig = &mHwL3CacheConfig;

    pCacheConfig->dwCntlReg = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12LP_RENDERHAL;

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
MOS_STATUS XRenderHal_Interface_g12::GetSamplerOffsetAndPtr_DSH(
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
    pDynamicState = pStateHeap->pCurMediaState->pDynamicState;

    MHW_RENDERHAL_CHK_NULL(pDynamicState);

    MHW_RENDERHAL_ASSERT(iMediaID   < pDynamicState->MediaID.iCount);

    dwOffset    = iMediaID * pDynamicState->dwSizeSamplers;                    // Go to Media ID sampler offset

    SamplerType = (pSamplerParams) ? pSamplerParams->SamplerType : MHW_SAMPLER_TYPE_3D;

    switch (SamplerType)
    {
        case MHW_SAMPLER_TYPE_AVS:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerAVS.iCount);
            dwOffset += pDynamicState->SamplerAVS.dwOffset +                    // Go to AVS sampler area
                        iSamplerID * MHW_SAMPLER_STATE_AVS_INC_G9;              // 16: size of one element, 128 elements for SKL
            break;

        case MHW_SAMPLER_TYPE_CONV:
            MHW_RENDERHAL_ASSERT(iSamplerID < pDynamicState->SamplerConv.iCount);
            dwOffset = pDynamicState->SamplerConv.dwOffset;                     // Goto Conv sampler base
            if ( pSamplerParams->Convolve.ui8ConvolveType == 0 && pSamplerParams->Convolve.skl_mode )
            {   // 2D convolve
                dwOffset += iSamplerID * MHW_SAMPLER_STATE_CONV_INC_G9;         // 16: size of one element, 128 elements for SKL
            }
            else if ( pSamplerParams->Convolve.ui8ConvolveType == 1 )
            {   // 1D convolve
                dwOffset += iSamplerID * MHW_SAMPLER_STATE_CONV_1D_INC_G9;      // 16: size of one element, 8 elements for SKL
            }
            else
            {   // 1P convolve (same as gen8) and 2D convolve BDW mode
                dwOffset += iSamplerID * MHW_SAMPLER_STATE_CONV_INC_G8;         // 16: size of one element, 32: 32 entry
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
void XRenderHal_Interface_g12::InitDynamicHeapSettings(
    PRENDERHAL_INTERFACE  pRenderHal)
{
    MHW_RENDERHAL_ASSERT(pRenderHal);

    // Additional Dynamic State Heap settings for g12
    pRenderHal->DynamicHeapSettings           = g_cRenderHal_DSH_Settings_g12;
}

void XRenderHal_Interface_g12::SetFusedEUDispatch(bool enable)
{
    m_vfeStateParams.bFusedEuDispatch = enable? true : false;
}

MOS_STATUS XRenderHal_Interface_g12::SetNumOfWalkers(uint32_t numOfWalkers)
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
MOS_STATUS XRenderHal_Interface_g12::SetPowerOptionStatus(
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
MOS_STATUS XRenderHal_Interface_g12::SetCompositePrologCmd(
    PRENDERHAL_INTERFACE pRenderHal, 
    PMOS_COMMAND_BUFFER  pCmdBuffer)
{
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    uint64_t                              auxTableBaseAddr = 0;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pMhwMiInterface);
    
    auxTableBaseAddr = pRenderHal->pOsInterface->pfnGetAuxTableBaseAddr(pRenderHal->pOsInterface);

    if (auxTableBaseAddr)
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS lriParams;
        MOS_ZeroMemory(&lriParams, sizeof(MHW_MI_LOAD_REGISTER_IMM_PARAMS));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseLow;
        lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, &lriParams));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioRcsAuxTableBaseHigh;
        lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, &lriParams));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseLow;
        lriParams.dwData = (auxTableBaseAddr & 0xffffffff);
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, &lriParams));

        lriParams.dwRegister = MhwMiInterfaceG12::m_mmioCcs0AuxTableBaseHigh;
        lriParams.dwData = ((auxTableBaseAddr >> 32) & 0xffffffff);
        MHW_RENDERHAL_CHK_STATUS(pRenderHal->pMhwMiInterface->AddMiLoadRegisterImmCmd(pCmdBuffer, &lriParams));

    }

finish:
    return eStatus;
}

MOS_STATUS XRenderHal_Interface_g12::IsRenderHalMMCEnabled(
    PRENDERHAL_INTERFACE         pRenderHal)
{
    MOS_STATUS                            eStatus = MOS_STATUS_SUCCESS;
    MOS_USER_FEATURE_VALUE_DATA           UserFeatureData;

    MHW_RENDERHAL_CHK_NULL_NO_STATUS(pRenderHal);

    // Read user feature key to set MMC for Fast Composition surfaces
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;
#ifdef LINUX
    UserFeatureData.bData       = false; // disable MMC on Linux
#else
    UserFeatureData.bData       = true; // init as default value to enable MMCD on Gen12LP
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_ENABLE_RENDER_ENGINE_MMC_ID,
        &UserFeatureData));
#endif

    m_renderHalMMCEnabled = UserFeatureData.bData && MEDIA_IS_SKU(pRenderHal->pSkuTable, FtrE2ECompression);
    pRenderHal->isMMCEnabled = m_renderHalMMCEnabled;

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
MOS_STATUS XRenderHal_Interface_g12::IsOvrdNeeded(
    PRENDERHAL_INTERFACE              pRenderHal,
    PMOS_COMMAND_BUFFER               pCmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS  pGenericPrologParams)
{
    PMOS_INTERFACE                        pOsInterface;
    MOS_STATUS                            eStatus;
    PMOS_CMD_BUF_ATTRI_VE                 pAttriVe;
    PRENDERHAL_GENERIC_PROLOG_PARAMS_G12  pGenericPrologParamsG12;
    uint8_t                               i;

    MHW_RENDERHAL_CHK_NULL(pRenderHal);
    MHW_RENDERHAL_CHK_NULL(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pOsInterface);
    MHW_RENDERHAL_CHK_NULL(pRenderHal->pRenderHalPltInterface);

    eStatus                 = MOS_STATUS_SUCCESS;
    pOsInterface            = pRenderHal->pOsInterface;
    pAttriVe               = (PMOS_CMD_BUF_ATTRI_VE)(pCmdBuffer->Attributes.pAttriVe);
    pGenericPrologParamsG12 = dynamic_cast<PRENDERHAL_GENERIC_PROLOG_PARAMS_G12>(pGenericPrologParams);

    // Split Frame
    if (pOsInterface->VEEnable)
    {
#if !EMUL
        if (pGenericPrologParamsG12)
#else
        if (pGenericPrologParamsG12 && pAttriVe != nullptr)
#endif
        {
            // Split Frame
            if (pGenericPrologParamsG12->VEngineHintParams.BatchBufferCount > 1)
            {
                pAttriVe->bUseVirtualEngineHint = true;
                pAttriVe->VEngineHintParams = pGenericPrologParamsG12->VEngineHintParams;
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
                if (pGenericPrologParamsG12)
#else
                if (pGenericPrologParamsG12 && pAttriVe != nullptr)
#endif
                {
                    pAttriVe->VEngineHintParams.BatchBufferCount = pGenericPrologParamsG12->VEngineHintParams.BatchBufferCount;
                    for (i = 0; i < pGenericPrologParamsG12->VEngineHintParams.BatchBufferCount; i++)
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
MOS_STATUS XRenderHal_Interface_g12::SetCacheOverrideParams(
    PRENDERHAL_INTERFACE            pRenderHal,
    PRENDERHAL_L3_CACHE_SETTINGS    pCacheSettings,
    bool                            bEnableSLM)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL(pCacheSettings);
    MHW_RENDERHAL_CHK_NULL(pRenderHal);

    pCacheSettings->dwCntlReg        = RENDERHAL_L3_CACHE_CONFIG_CNTLREG_VALUE_G12LP_RENDERHAL;
    pCacheSettings->bCntlRegOverride = true;

finish:
    return eStatus;
}

//! \brief      Get the size of Render Surface State Command
//! \return     size_t
//!             the size of render surface state command
size_t XRenderHal_Interface_g12::GetSurfaceStateCmdSize()
{
    return MOS_ALIGN_CEIL( MOS_MAX(mhw_state_heap_g12_X::RENDER_SURFACE_STATE_CMD::byteSize,
                   mhw_state_heap_g12_X::MEDIA_SURFACE_STATE_CMD::byteSize), MHW_SURFACE_STATE_ALIGN);
}

//! \brief      Get Surface Compression support caps
//! \param      [in] format
//!             surface format
//! \return     bool
//!             true or false
bool XRenderHal_Interface_g12::IsFormatMMCSupported(MOS_FORMAT format)
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
        (format != Format_R8G8UN))
    {
        MHW_RENDERHAL_NORMALMESSAGE("Unsupported Format '0x%08x' for Render MMC.", format);
        return false;
    }

    return true;
}

MOS_STATUS XRenderHal_Interface_g12::AllocateScratchSpaceBuffer(
    uint32_t perThreadScratchSpace,
    RENDERHAL_INTERFACE *renderHal)
{
    return MOS_STATUS_UNIMPLEMENTED;
}

static const uint32_t FIXED_SCRATCH_SPACE_BUFFER_INDEX = 6;

MOS_STATUS XRenderHal_Interface_g12::SetScratchSpaceBufferState(
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
    renderhal_surface_state_param.bRenderTarget = 1;
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

MOS_STATUS XRenderHal_Interface_g12::FreeScratchSpaceBuffer(
    RENDERHAL_INTERFACE *renderHal)
{
    if (m_scratchSpaceResource.iSize <= 0)
    {
        return MOS_STATUS_SUCCESS;  // Scratch space is not allocated. No need to free resources.
    }

    renderHal->pOsInterface
            ->pfnFreeResourceWithFlag(renderHal->pOsInterface,
                                      &m_scratchSpaceResource,
                                      1);
    renderHal->pOsInterface
            ->pfnResetResourceAllocationIndex(renderHal->pOsInterface,
                                              &m_scratchSpaceResource);
    return MOS_STATUS_SUCCESS;
}
