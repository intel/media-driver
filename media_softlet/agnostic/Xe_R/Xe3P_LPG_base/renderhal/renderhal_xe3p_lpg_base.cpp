/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file       renderhal_xe3p_lpg_base.cpp
//! \brief      implementation of xe3p_lpg_base hardware functions
//! \details    Render functions
//!

#include "renderhal_xe3p_lpg_base.h"
#include "vp_utils.h"
#include "media_packet.h"
#include "media_feature.h"

//!
//! \brief      GSH settings for Xe3p_lpg_base
//!
#define RENDERHAL_SAMPLERS_AVS_XE3P_LPG_BASE          0

#define RENDERHAL_SSH_SURFACES_PER_BT_XE3P_LPG_BASE 80
#define ENLARGE_KERNEL_COUNT_XE3P_LPG_BASE          RENDERHAL_KERNEL_COUNT * 3
#define ENLARGE_KERNEL_HEAP_XE3P_LPG_BASE           RENDERHAL_KERNEL_HEAP * 3
#define ENLARGE_CURBE_SIZE_XE3P_LPG_BASE            RENDERHAL_CURBE_SIZE * 16
#define ENLARGED_SSH_BINDING_TABLES_XE3P_LPG_BASE   16
#define ENLARGED_MEDIA_IDS_XE3P_LPG_BASE            32

extern const RENDERHAL_STATE_HEAP_SETTINGS g_cRenderHal_State_Heap_Settings_xe3p_lpg_base =
{
    // Global GSH Allocation parameters
    RENDERHAL_SYNC_SIZE,                       //!< iSyncSize

    // Media State Allocation parameters
    RENDERHAL_MEDIA_STATES,                    //!< iMediaStateHeaps - Set by Initialize
    RENDERHAL_MEDIA_IDS,                       //!< iMediaIDs
    RENDERHAL_CURBE_SIZE,                      //!< iCurbeSize
    RENDERHAL_SAMPLERS,                        //!< iSamplers
    RENDERHAL_SAMPLERS_AVS_XE3P_LPG_BASE,      //!< iSamplersAVS
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
    MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER //!< heapUsageType
};

extern const RENDERHAL_ENLARGE_PARAMS g_cRenderHal_Enlarge_State_Heap_Settings_Adv_xe3p_lpg_base =
{
    ENLARGED_SSH_BINDING_TABLES_XE3P_LPG_BASE,    //!< iBindingTables
    RENDERHAL_SSH_SURFACE_STATES_MAX,            //!< iSurfaceStates
    RENDERHAL_SSH_SURFACES_PER_BT_XE3P_LPG_BASE,  //!< iSurfacesPerBT
    ENLARGE_KERNEL_COUNT_XE3P_LPG_BASE,          //!< iKernelCount
    ENLARGE_KERNEL_HEAP_XE3P_LPG_BASE,           //!< iKernelHeapSize
    ENLARGE_CURBE_SIZE_XE3P_LPG_BASE,            //!< iCurbeSize
    ENLARGED_MEDIA_IDS_XE3P_LPG_BASE             //!< iMediaIDs
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
MOS_STATUS XRenderHal_Interface_Xe3P_Lpg_Base::SetupSurfaceState(
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
        pSurfaceEntry->dwSurfStateOffset = pSurfaceEntry->iSurfStateID * dwSurfaceSize;   // no binding table

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
        SurfStateParams.RotationMode          = g_cLookup_RotationMode_Next[pRenderHalSurface->Rotation];
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

//!
//! \brief    Send To 3DState Binding Table Pool Alloc
//! \details  Send To 3DState Binding Table Pool Alloc
//! \param    PRENDERHAL_INTERFACE pRenderHal
//!           [in] Pointer to RenderHal Interface Structure
//! \param    PMOS_COMMAND_BUFFER pCmdBuffer
//!           [in] Pointer to Command Buffer
//! \return   MOS_STATUS
//!
MOS_STATUS XRenderHal_Interface_Xe3P_Lpg_Base::SendTo3DStateBindingTablePoolAlloc(
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
    m_renderItf->MHW_ADDCMD_F(STATE_COMPUTE_MODE)(pCmdBuffer);

    SETPAR_AND_ADDCMD(_3DSTATE_BINDING_TABLE_POOL_ALLOC, m_renderItf, pCmdBuffer);

    return eStatus;
}

MOS_STATUS XRenderHal_Interface_Xe3P_Lpg_Base::SendStateComputeMode(
    PRENDERHAL_INTERFACE pRenderHal,
    PMOS_COMMAND_BUFFER  pCmdBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_RENDERHAL_CHK_NULL_RETURN(pRenderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(pCmdBuffer);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderItf);

    auto &computerModeParams                       = m_renderItf->MHW_GETPAR_F(STATE_COMPUTE_MODE)();
    computerModeParams                             = {};
    computerModeParams.enableLargeGrf              = pRenderHal->largeGrfMode;
    computerModeParams.forceEuThreadSchedulingMode = pRenderHal->euThreadSchedulingMode;
    m_renderItf->MHW_ADDCMD_F(STATE_COMPUTE_MODE)(pCmdBuffer);

    return eStatus;
}

//!
//! \brief    Initialize the State Heap Settings per platform
//! \param    PRENDERHAL_STATE_HEAP_SETTINGS pSettings
//!           [out] Pointer to PRENDERHAL_STATE_HEAP_SETTINGSStructure
//! \return   void
//!
void XRenderHal_Interface_Xe3P_Lpg_Base::InitStateHeapSettings(
    PRENDERHAL_INTERFACE    pRenderHal)
{
    MHW_RENDERHAL_CHK_NULL_NO_STATUS_RETURN(pRenderHal);
    // Set State Heap settings for xe3p_lpg_base
    pRenderHal->StateHeapSettings              = g_cRenderHal_State_Heap_Settings_xe3p_lpg_base;
    pRenderHal->defaultStateHeapSettings       = g_cRenderHal_State_Heap_Settings_xe3p_lpg_base;
    pRenderHal->enlargeStateHeapSettingsForAdv = g_cRenderHal_Enlarge_State_Heap_Settings_Adv_xe3p_lpg_base;
}

MHW_SETPAR_DECL_SRC(CFE_STATE, XRenderHal_Interface_Xe3P_Lpg_Base)
{
    // Remove CFE STATE
    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(COMPUTE_WALKER, XRenderHal_Interface_Xe3P_Lpg_Base)
{
    MHW_VFE_PARAMS* pVfeStateParams     = nullptr;
    MHW_VFE_PARAMS_NEXT* paramsNext   = nullptr;
    pVfeStateParams                     = nullptr;

    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal->pHwCaps);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal->pRenderHalPltInterface);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_renderHal->pRenderHalPltInterface->GetVfeStateParameters());

    pVfeStateParams     = m_renderHal->pRenderHalPltInterface->GetVfeStateParameters();
    params.cfeState.pKernelState = pVfeStateParams->pKernelState;

    if (pVfeStateParams->pKernelState)
    {
        params.cfeState.dwMaximumNumberofThreads = (pVfeStateParams->dwMaximumNumberofThreads) ? pVfeStateParams->dwMaximumNumberofThreads - 1 : pVfeStateParams->pKernelState->KernelParams.iThreadCount - 1;
    }
    else
    {
        params.cfeState.dwMaximumNumberofThreads = (pVfeStateParams->dwMaximumNumberofThreads) ? pVfeStateParams->dwMaximumNumberofThreads - 1 : m_renderHal->pHwCaps->dwMaxThreads - 1;
    }

    paramsNext = dynamic_cast<MHW_VFE_PARAMS_NEXT*>(pVfeStateParams);
    if (paramsNext != nullptr)
    {
        params.cfeState.FusedEuDispatch   = paramsNext->bFusedEuDispatch ? false : true;  // disabled if DW3.FusedEuDispath = 1
        params.cfeState.NumberOfWalkers   = paramsNext->numOfWalkers;
        params.cfeState.SingleSliceDispatchCcsMode = paramsNext->enableSingleSliceDispatchCcsMode;
    }

    // Compute WALKER Parameters
    MHW_RENDERHAL_CHK_NULL_RETURN(m_gpgpuWalkerParams);
    MHW_RENDERHAL_CHK_NULL_RETURN(m_interfaceDescriptorParams);

    params.simdSize                 = m_gpgpuWalkerParams->simdSize;
    params.ThreadWidth              = m_gpgpuWalkerParams->ThreadWidth;
    params.ThreadHeight             = m_gpgpuWalkerParams->ThreadHeight;
    params.ThreadDepth              = m_gpgpuWalkerParams->ThreadDepth;

    params.GroupWidth     = m_gpgpuWalkerParams->GroupWidth;
    params.GroupHeight    = m_gpgpuWalkerParams->GroupHeight;
    params.GroupDepth     = m_gpgpuWalkerParams->GroupDepth;
    params.GroupStartingX = m_gpgpuWalkerParams->GroupStartingX;
    params.GroupStartingY = m_gpgpuWalkerParams->GroupStartingY;
    params.GroupStartingZ = m_gpgpuWalkerParams->GroupStartingZ;

    params.dwKernelOffset       = m_interfaceDescriptorParams->dwKernelOffset;
    params.kernelSize           = m_interfaceDescriptorParams->kernelSize;

    params.bBarrierEnable                = m_interfaceDescriptorParams->bBarrierEnable;
    params.dwNumberofThreadsInGPGPUGroup = m_interfaceDescriptorParams->dwNumberofThreadsInGPGPUGroup;
    params.dwSharedLocalMemorySize       = m_interfaceDescriptorParams->dwSharedLocalMemorySize;
    params.preferredSlmAllocationSize    = m_interfaceDescriptorParams->preferredSlmAllocationSize;
    params.forcePreferredSLMZero         = m_gpgpuWalkerParams->ForcePreferredSLMZero;

    if (m_gpgpuWalkerParams->ThreadDepth == 0)
    {
        params.ThreadDepth = 1;
    }
    if (m_gpgpuWalkerParams->GroupDepth == 0)
    {
        params.GroupDepth = 1;
    }
    if (m_gpgpuWalkerParams->simdSize == 0)
    {
        params.simdSize = 32;
    }

    params.registersPerThread    = m_gpgpuWalkerParams->registersPerThread;
    params.isEmitInlineParameter = m_gpgpuWalkerParams->isEmitInlineParameter;
    if (m_gpgpuWalkerParams->inlineDataLength > 0 && m_gpgpuWalkerParams->inlineData)
    {
        params.inlineDataLength = m_gpgpuWalkerParams->inlineDataLength;
        params.inlineData       = m_gpgpuWalkerParams->inlineData;
    }

    params.isGenerateLocalId = m_gpgpuWalkerParams->isGenerateLocalID;
    params.emitLocal         = mhw::render::MHW_EMIT_LOCAL_MODE(m_gpgpuWalkerParams->emitLocal);


    MHW_STATE_BASE_ADDR_PARAMS *pStateBaseParams = &m_renderHal->StateBaseAddressParams;
    MHW_RENDERHAL_CHK_NULL_RETURN(pStateBaseParams);
    MHW_RENDERHAL_CHK_VALUE_RETURN(Mos_ResourceIsNull(pStateBaseParams->presInstructionBuffer), false);
    params.heapsResource.presInstructionBuffer  = pStateBaseParams->presInstructionBuffer;
    params.heapsResource.instructionBufferPtr   = m_renderHal->pStateHeap ? m_renderHal->pStateHeap->pIshBuffer : nullptr;
    params.heapsResource.curbeResourceList      = m_gpgpuWalkerParams->curbeResourceList;
    params.heapsResource.curbeResourceListSize  = m_gpgpuWalkerParams->curbeResourceListSize;
    params.heapsResource.inlineResourceList     = m_gpgpuWalkerParams->inlineResourceList;
    params.heapsResource.inlineResourceListSize = m_gpgpuWalkerParams->inlineResourceListSize;

    return MOS_STATUS_SUCCESS;
}