/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     media_render_common.c
//! \brief    The file of common utilities definitions shared by low level renderers
//! \details  Common utilities for different renderers
//!
#include "media_render_common.h"
#include "hal_oca_interface_next.h"
#include "renderhal_platform_interface.h"
//!
//! \brief    Determine if the Batch Buffer End is needed to add in the end
//! \details  Detect platform OS and return the flag whether the Batch Buffer End is needed to add in the end
//! \param    [in] pOsInterface
//!           Pointer to MOS_INTERFACE
//! \return   bool
//!           The flag of adding Batch Buffer End
//!
static bool IsMiBBEndNeeded(
    PMOS_INTERFACE           pOsInterface)
{
    bool needed = true;
    return needed;
}

//!
//! \brief    init render hal surface.
//! \details  fill render hal surface's paramters
//! \param    [in] pSurface
//!           Pointer to PMOS_SURFACE
//! \param    [in] pRenderHalSurface
//!           Pointer to PRENDERHAL_SURFACE
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
static MOS_STATUS InitRenderHalSurface(
    PMOS_INTERFACE          pOsInterface,
    PMOS_SURFACE            pSurface,
    PRENDERHAL_SURFACE      pRenderHalSurface)
{
    MOS_STATUS              eStatus = MOS_STATUS_SUCCESS;

    if (nullptr == pSurface || nullptr == pOsInterface || nullptr == pRenderHalSurface)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    MOS_ZeroMemory(pRenderHalSurface, sizeof(*pRenderHalSurface));

    pRenderHalSurface->OsSurface.OsResource = pSurface->OsResource;
    pRenderHalSurface->OsSurface.dwWidth = pSurface->dwWidth;
    pRenderHalSurface->OsSurface.dwHeight = pSurface->dwHeight;
    pRenderHalSurface->OsSurface.dwPitch = pSurface->dwPitch;
    pRenderHalSurface->OsSurface.Format = pSurface->Format;
    pRenderHalSurface->OsSurface.TileType = pSurface->TileType;
    pRenderHalSurface->OsSurface.TileModeGMM = pSurface->TileModeGMM;
    pRenderHalSurface->OsSurface.bGMMTileEnabled = pSurface->bGMMTileEnabled;
    pRenderHalSurface->OsSurface.dwOffset = pSurface->dwOffset;
    pRenderHalSurface->OsSurface.bIsCompressed = pSurface->bIsCompressed;
    pRenderHalSurface->OsSurface.bCompressible = pSurface->bCompressible;
    pRenderHalSurface->OsSurface.CompressionMode = pSurface->CompressionMode;
    pRenderHalSurface->OsSurface.dwDepth = pSurface->dwDepth;
    pRenderHalSurface->OsSurface.dwQPitch = pSurface->dwHeight;
    pRenderHalSurface->OsSurface.MmcState = (MOS_MEMCOMP_STATE)pSurface->CompressionMode;
    pRenderHalSurface->OsSurface.CompressionFormat = pSurface->CompressionFormat;

    pRenderHalSurface->OsSurface.YPlaneOffset = pSurface->YPlaneOffset;
    pRenderHalSurface->OsSurface.UPlaneOffset = pSurface->UPlaneOffset;
    pRenderHalSurface->OsSurface.VPlaneOffset = pSurface->VPlaneOffset;
    pRenderHalSurface->SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;

    MOS_SURFACE    ResDetails = {0};
    MHW_RENDERHAL_ASSERT(!Mos_ResourceIsNull(&pSurface->OsResource));
    ResDetails.Format = pSurface->Format;
    MHW_CHK_STATUS_RETURN(pOsInterface->pfnGetResourceInfo(pOsInterface, &pSurface->OsResource, &ResDetails));

    pRenderHalSurface->rcSrc.bottom = pSurface->dwHeight;
    pRenderHalSurface->rcSrc.right = ResDetails.dwWidth;
    pRenderHalSurface->rcDst.bottom = pSurface->dwHeight;
    pRenderHalSurface->rcDst.right = ResDetails.dwWidth;
    pRenderHalSurface->rcMaxSrc.bottom = pSurface->dwHeight;
    pRenderHalSurface->rcMaxSrc.right = ResDetails.dwWidth;
    pRenderHalSurface->OsSurface.dwQPitch = pSurface->dwHeight;

    return MOS_STATUS_SUCCESS;
}
//!
//! \brief    Set 2D Surface for HW Access
//! \details  Common Function for setting up buffer surface state, need to use this function
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to bind surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS MediaRenderCommon::Set2DSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PMOS_SURFACE                        pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{

    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES] = {0};
    int32_t                         iSurfaceEntries = 0;
    int32_t                         i = 0;
    MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;

    if (nullptr == pRenderHal || nullptr == pRenderHal->pOsInterface || nullptr == pRenderSurface || nullptr == pSurface)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    // Initialize Variables
    pOsInterface = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true);

    RENDERHAL_GET_SURFACE_INFO info;
    MOS_ZeroMemory(&info, sizeof(info));
    MHW_CHK_STATUS_RETURN(RenderHal_GetSurfaceInfo(
        pRenderHal->pOsInterface,
        &info,
        pSurface));

    pRenderSurface->OsSurface = *pSurface;
    pRenderSurface->rcSrc.bottom       = pSurface->dwHeight;
    pRenderSurface->rcSrc.right        = pSurface->dwWidth;
    pRenderSurface->rcDst.bottom       = pSurface->dwHeight;
    pRenderSurface->rcDst.right        = pSurface->dwWidth;
    pRenderSurface->rcMaxSrc.bottom    = pSurface->dwHeight;
    pRenderSurface->rcMaxSrc.right     = pSurface->dwWidth;
    pRenderSurface->OsSurface.dwQPitch = pSurface->dwHeight;

    if (bWrite)
    {
        pRenderSurface->SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;

        // Widthscalar is 2 for RENDERHAL_PLANES_Y210_RT (Plane definition) layout
        if (pRenderSurface->OsSurface.Format == Format_Y210 || pRenderSurface->OsSurface.Format == Format_Y216)
        {
            pRenderSurface->rcDst.right       = pSurface->dwWidth * 2;
            pRenderSurface->OsSurface.dwWidth = pSurface->dwWidth * 2;
        }
    }

    // Setup surface states-----------------------------------------------------
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnSetupSurfaceState(
        pRenderHal,
        pRenderSurface,
        pSurfaceParams,
        &iSurfaceEntries,
        pSurfaceEntries,
        nullptr));

    // Bind surface states------------------------------------------------------
    for (i = 0; i < iSurfaceEntries; i++, iBTEntry++)
    {
        MHW_CHK_STATUS_RETURN(pRenderHal->pfnBindSurfaceState(
            pRenderHal,
            iBindingTable,
            iBTEntry,
            pSurfaceEntries[i]));
    }

    return eStatus;
}

//!
//! \brief    Set 1D Surface for HW Access
//! \details  Common Function for setting up buffer surface state, need to use this function
//! \param    [in] pRenderHal
//!           Pointer to RenderHal Interface Structure
//! \param    [in] pSurface
//!           Pointer to Surface
//! \param    [in] pRenderSurface
//!           Pointer to Render Surface
//! \param    [in,out] pSurfaceParams
//!           Pointer to RenderHal Surface Params
//! \param    [in] iBindingTable
//!           Binding Table to Bind Surface
//! \param    [in] iBTEntry
//!           Binding Table Entry index
//! \param    [in] bWrite
//!           Write mode flag
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success. Error code otherwise
//!
MOS_STATUS MediaRenderCommon::Set1DSurfaceForHwAccess(
    PRENDERHAL_INTERFACE                pRenderHal,
    PMOS_SURFACE                        pSurface,
    PRENDERHAL_SURFACE                  pRenderSurface,
    PRENDERHAL_SURFACE_STATE_PARAMS     pSurfaceParams,
    int32_t                             iBindingTable,
    int32_t                             iBTEntry,
    bool                                bWrite)
{
    PMOS_INTERFACE                      pOsInterface;
    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntry;
    MOS_FORMAT                          tempformat = Format_Any;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    if (nullptr == pRenderHal || nullptr == pRenderHal->pOsInterface || nullptr == pRenderSurface || nullptr == pSurface)
    {
        return MOS_STATUS_NULL_POINTER;
    }

    // Initialize Variables
    eStatus = MOS_STATUS_SUCCESS;
    pOsInterface = pRenderHal->pOsInterface;

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    MHW_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(
        pOsInterface,
        &pSurface->OsResource,
        bWrite,
        true));

    // Setup Buffer surface-----------------------------------------------------
    if (pSurfaceParams == nullptr)
    {
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));

        //set mem object control for cache
        SurfaceParam.MemObjCtl = (pRenderHal->pOsInterface->pfnCachePolicyGetMemoryObject(
            MOS_HW_RESOURCE_USAGE_VP_INTERNAL_READ_WRITE_RENDER,
            pRenderHal->pOsInterface->pfnGetGmmClientContext(pRenderHal->pOsInterface))).DwordValue;

        pSurfaceParams = &SurfaceParam;
    }

    MHW_CHK_STATUS_RETURN(InitRenderHalSurface(pOsInterface, pSurface, pRenderSurface));
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnSetupBufferSurfaceState(
        pRenderHal,
        pRenderSurface,
        pSurfaceParams,
        &pSurfaceEntry));

    // Bind surface state-------------------------------------------------------
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnBindSurfaceState(
        pRenderHal,
        iBindingTable,
        iBTEntry,
        pSurfaceEntry));

    return eStatus;
}

MOS_STATUS MediaRenderCommon::SetPowerMode(
    PRENDERHAL_INTERFACE              pRenderHal,
    uint32_t                          KernelID)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint16_t                            wNumRequestedEUSlices = 1;    // Default to 1 slice
    uint16_t                            wNumRequestedSubSlices = 3;    // Default to 3 subslice
    uint16_t                            wNumRequestedEUs = 8;    // Default to 8 EUs
    RENDERHAL_POWEROPTION               PowerOption;
    bool                                bSetRequestedSlices = false;
    const euSetting                    *pcSSEUTable = nullptr;
    MediaUserSettingSharedPtr           userSettingPtr = nullptr;
    uint32_t                            value = 0;

    MHW_CHK_NULL_RETURN(pRenderHal);

    if ((pRenderHal->bRequestSingleSlice) || (pRenderHal->bEUSaturationNoSSD))
    {
        bSetRequestedSlices = true;
        // bEUSaturationNoSSD: No slice shutdown, must request 2 slices [CM EU saturation on].
        // bRequestSingleSlice: Always single slice.
        wNumRequestedEUSlices = (pRenderHal->bEUSaturationNoSSD) ? 2 : 1;
    }
    else
    {
        bSetRequestedSlices = false;
    }

    if (pRenderHal->sseuTable)
    {
        pcSSEUTable = (const euSetting*)pRenderHal->sseuTable;
    }
    else
    {
        MHW_ASSERTMESSAGE("SSEU Table not valid.");
        eStatus = MOS_STATUS_UNKNOWN;
        return eStatus;
    }

    MHW_CHK_NULL_RETURN(pcSSEUTable);
    pcSSEUTable += KernelID;
    if (!bSetRequestedSlices)                        // If num Slices is already programmed, then don't change it
    {
        if (wNumRequestedEUSlices < pcSSEUTable->numSlices)
        {
            wNumRequestedEUSlices = pcSSEUTable->numSlices;
        }
    }

    wNumRequestedSubSlices = pcSSEUTable->numSubSlices;
    wNumRequestedEUs = pcSSEUTable->numEUs;

#if (_DEBUG || _RELEASE_INTERNAL)
    // User feature key reads
    userSettingPtr = pRenderHal->pOsInterface->pfnGetUserSettingInstance(pRenderHal->pOsInterface);
    ReadUserSettingForDebug(
        userSettingPtr,
        value,
        __MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE,
        MediaUserSetting::Group::Device);
    if (value != 0xDEADC0DE)
    {
        wNumRequestedEUSlices = value & 0xFF;               // Bits 0-7
        wNumRequestedSubSlices = (value >> 8) & 0xFF;        // Bits 8-15
        wNumRequestedEUs = (value >> 16) & 0xFFFF;     // Bits 16-31
    }
#endif

    PowerOption.nSlice = wNumRequestedEUSlices;
    PowerOption.nSubSlice = wNumRequestedSubSlices;
    PowerOption.nEU = wNumRequestedEUs;
    pRenderHal->pfnSetPowerOptionMode(pRenderHal, &PowerOption);

    return eStatus;
}

//!
//! \brief      Submit commands for rendering
//! \details    Submit commands for rendering. The KMD related fields in pGenericPrologParam might be modified by this
//!             function in order to maintain the synchronization mechanism for resource.
//! \param      [in] pRenderHal
//!             Pointer to RenderHal Interface Structure
//! \param      [in] pBatchBuffer
//!             Pointer to batch buffer
//! \param      [in] bNullRendering
//!             Indicate whether is Null rendering
//! \param      [in] pWalkerParams
//!             Pointer to walker parameters
//! \param      [in] pGpGpuWalkerParams
//!             Pointer to GPGPU walker parameters
//! \param      [in] KernelID
//!             VP Kernel ID
//! \param      [in] bLastSubmission
//!             Is last submission
//! \return     MOS_STATUS
//!
MOS_STATUS MediaRenderCommon::EukernelSubmitCommands(
    PRENDERHAL_INTERFACE                pRenderHal,
    PMHW_BATCH_BUFFER                   pBatchBuffer,
    bool                                bNullRendering,
    PMHW_WALKER_PARAMS                  pWalkerParams,
    PMHW_GPGPU_WALKER_PARAMS            pGpGpuWalkerParams,
    uint32_t                            KernelID,
    bool                                bLastSubmission)
{
    PMOS_INTERFACE                      pOsInterface = nullptr;
    MOS_COMMAND_BUFFER                  CmdBuffer = {};
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint32_t                            dwSyncTag = 0;
    int32_t                             i = 0, iRemaining = 0;
    std::shared_ptr<mhw::render::Itf>   renderItf = nullptr;

    MHW_MEDIA_STATE_FLUSH_PARAM         FlushParam = {};
    bool                                bEnableSLM = false;
    RENDERHAL_GENERIC_PROLOG_PARAMS     GenericPrologParams = {};
    MOS_RESOURCE                       *pgpuStatusBuffer= nullptr;
    MOS_CONTEXT                        *pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS               pMmioRegisters = nullptr;
    MHW_FUNCTION_ENTER;

    MHW_CHK_NULL_RETURN(pRenderHal);
    MHW_CHK_NULL_RETURN(pRenderHal->pRenderHalPltInterface);
    MHW_CHK_NULL_RETURN(pRenderHal->pOsInterface);
    MHW_CHK_NULL_RETURN(pRenderHal->pOsInterface->pOsContext);

    pOsInterface = pRenderHal->pOsInterface;
   
    iRemaining = 0;
    FlushParam = g_cRenderHal_InitMediaStateFlushParams;
    MOS_ZeroMemory(&CmdBuffer, sizeof(CmdBuffer));
    pOsContext = pOsInterface->pOsContext;
    pMmioRegisters = pRenderHal->pRenderHalPltInterface->GetMmioRegisters(pRenderHal);
    MHW_CHK_NULL_RETURN(pMmioRegisters);
    // Allocate all available space, unused buffer will be returned later
    MHW_CHK_STATUS_RETURN(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBuffer, 0));

    // Set initial state
    iRemaining = CmdBuffer.iRemaining;

    MHW_CHK_STATUS_RETURN(SetPowerMode(pRenderHal,KernelID));
    pRenderHal->pRenderHalPltInterface->On1stLevelBBStart(pRenderHal, &CmdBuffer, pOsContext, pOsInterface->CurrentGpuContextHandle, pMmioRegisters);

#ifndef EMUL
    if (bLastSubmission && pOsInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, pgpuStatusBuffer);
        // Register the buffer
        pOsInterface->pfnRegisterResource(pOsInterface, pgpuStatusBuffer, true, true);

        GenericPrologParams.bEnableMediaFrameTracking = true;
        GenericPrologParams.presMediaFrameTrackingSurface = pgpuStatusBuffer;
        GenericPrologParams.dwMediaFrameTrackingTag = pOsInterface->pfnGetGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
        GenericPrologParams.dwMediaFrameTrackingAddrOffset = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);

        // Increment GPU Status Tag
        pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, pOsInterface->CurrentGpuContextOrdinal);
    }
#endif

    // Initialize command buffer and insert prolog
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnInitCommandBuffer(pRenderHal, &CmdBuffer, &GenericPrologParams));

    // Write timing data for 3P budget
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnSendTimingData(pRenderHal, &CmdBuffer, true));

    MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddPerfCollectStartCmd(pRenderHal, pOsInterface, &CmdBuffer));

    MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->StartPredicate(pRenderHal, &CmdBuffer));

    bEnableSLM = (pGpGpuWalkerParams && pGpGpuWalkerParams->SLMSize > 0) ? true : false;
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnSetCacheOverrideParams(
        pRenderHal,
        &pRenderHal->L3CacheSettings,
        bEnableSLM));

    // Flush media states
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnSendMediaStates(
        pRenderHal,
        &CmdBuffer,
        pWalkerParams,
        pGpGpuWalkerParams));


    // Write back GPU Status tag
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        MHW_CHK_STATUS_RETURN(pRenderHal->pfnSendRcsStatusTag(pRenderHal, &CmdBuffer));
    }

    pRenderHal->pRenderHalPltInterface->StopPredicate(pRenderHal, &CmdBuffer);

    MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddPerfCollectEndCmd(pRenderHal, pOsInterface, &CmdBuffer));


    // Write timing data for 3P budget
    MHW_CHK_STATUS_RETURN(pRenderHal->pfnSendTimingData(pRenderHal, &CmdBuffer, false));


    MHW_PIPE_CONTROL_PARAMS PipeControlParams;

    MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
    PipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    PipeControlParams.bGenericMediaStateClear = true;
    PipeControlParams.bIndirectStatePointersDisable = true;
    PipeControlParams.bDisableCSStall = false;

    MHW_CHK_NULL_RETURN(pOsInterface->pfnGetSkuTable);
    auto* skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
    {
        // Add PPC fulsh
        PipeControlParams.bPPCFlush = true;
    }
    MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddMiPipeControl(pRenderHal, &CmdBuffer, &PipeControlParams));

    if (MEDIA_IS_WA(pRenderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
    {
        MHW_VFE_PARAMS VfeStateParams = {};
        VfeStateParams.dwNumberofURBEntries = 1;
        MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddMediaVfeCmd(pRenderHal, &CmdBuffer, &VfeStateParams));
    }

    // Add media flush command in case HW not cleaning the media state
    if (MEDIA_IS_WA(pRenderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
    {
        FlushParam.bFlushToGo = true;
        if (pWalkerParams)
        {
            FlushParam.ui8InterfaceDescriptorOffset = pWalkerParams->InterfaceDescriptorOffset;
        }
        else
        {
            MHW_ASSERTMESSAGE("ERROR, pWalkerParams is nullptr and cannot get InterfaceDescriptorOffset.");
        }

        MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddMediaStateFlush(pRenderHal, &CmdBuffer, &FlushParam));
    }
    else if (MEDIA_IS_WA(pRenderHal->pWaTable, WaAddMediaStateFlushCmd))
    {
        MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddMediaStateFlush(pRenderHal, &CmdBuffer, &FlushParam));
    }


    HalOcaInterfaceNext::On1stLevelBBEnd(CmdBuffer, *pOsInterface);

    if (pBatchBuffer)
    {
        // Send Batch Buffer end command (HW/OS dependent)
        MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddMiBatchBufferEnd(pRenderHal, &CmdBuffer, nullptr));
    }
    else if (IsMiBBEndNeeded(pOsInterface))
    {
        // Send Batch Buffer end command for 1st level Batch Buffer
        MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddMiBatchBufferEnd(pRenderHal, &CmdBuffer, nullptr));
    }
    else if (pRenderHal->pOsInterface->bNoParsingAssistanceInKmd)
    {
        MHW_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddMiBatchBufferEnd(pRenderHal, &CmdBuffer, nullptr));
    }

    // Return unused command buffer space to OS
    pOsInterface->pfnReturnCommandBuffer(pOsInterface, &CmdBuffer, 0);



    // Submit command buffer
    MHW_CHK_STATUS_RETURN(pOsInterface->pfnSubmitCommandBuffer(pOsInterface, &CmdBuffer, bNullRendering));

    if (bNullRendering == false)
    {
        dwSyncTag = pRenderHal->pStateHeap->dwNextTag++;

        // Set media state and batch buffer as busy
        pRenderHal->pStateHeap->pCurMediaState->bBusy = true;
        if (pBatchBuffer)
        {
            pBatchBuffer->bBusy = true;
            pBatchBuffer->dwSyncTag = dwSyncTag;
        }
    }

    return eStatus;
}
