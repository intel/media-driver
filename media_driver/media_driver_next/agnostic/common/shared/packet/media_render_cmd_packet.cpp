/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     media_render_cmd_packet.cpp
//! \brief    Defines the interface for media render workload cmd packet
//! \details  The media cmd packet is dedicated for command buffer sequenece submit
//!

#include "media_render_cmd_packet.h"
#include "mos_oca_interface.h"

RenderCmdPacket::RenderCmdPacket(MediaTask* task, PMOS_INTERFACE pOsinterface, RENDERHAL_INTERFACE *renderHal) : CmdPacket(task),
m_renderHal(renderHal),
m_cpInterface(nullptr),
m_osInterface(pOsinterface)
{
    Init();
}

RenderCmdPacket::~RenderCmdPacket()
{
    Destroy();
}

MOS_STATUS RenderCmdPacket::Init()
{
    if (!m_renderHal)
    {
        m_renderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(RENDERHAL_INTERFACE));
        RENDER_PACKET_CHK_NULL_RETURN(m_renderHal);
        RENDER_PACKET_CHK_STATUS_RETURN(RenderHal_InitInterface(
            m_renderHal,
            &m_cpInterface,
            m_osInterface));

        RENDERHAL_SETTINGS          RenderHalSettings;
        RenderHalSettings.iMediaStates = 32; // Init MEdia state values
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));
    }
    else
    {
        RENDER_PACKET_NORMALMESSAGE("RenderHal Already been created");
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::Destroy()
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_kernelEntry)
    {
        MOS_FreeMemory(m_kernelEntry);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase)
{
    PMOS_INTERFACE                      pOsInterface = nullptr;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint32_t                            dwSyncTag = 0;
    int32_t                             i = 0, iRemaining = 0;
    PMHW_MI_INTERFACE                   pMhwMiInterface = nullptr;
    MhwRenderInterface*                 pMhwRender = nullptr;
    MHW_MEDIA_STATE_FLUSH_PARAM         FlushParam = {};
    bool                                bEnableSLM = false;
    RENDERHAL_GENERIC_PROLOG_PARAMS     GenericPrologParams = {};
    MOS_RESOURCE                        GpuStatusBuffer = {};
    MediaPerfProfiler*                  pPerfProfiler = nullptr;
    MOS_CONTEXT*                        pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS               pMmioRegisters = nullptr;

    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pMhwRenderInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pMhwMiInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pMhwRenderInterface->GetMmioRegisters());
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pOsInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pOsInterface->pOsContext);

    eStatus = MOS_STATUS_UNKNOWN;
    pOsInterface = m_renderHal->pOsInterface;
    pMhwMiInterface = m_renderHal->pMhwMiInterface;
    pMhwRender = m_renderHal->pMhwRenderInterface;
    iRemaining = 0;
    FlushParam = g_cRenderHal_InitMediaStateFlushParams;
    MOS_ZeroMemory(commandBuffer, sizeof(MOS_COMMAND_BUFFER));
    pPerfProfiler = m_renderHal->pPerfProfiler;
    pOsContext = pOsInterface->pOsContext;
    pMmioRegisters = pMhwRender->GetMmioRegisters();

    RENDER_PACKET_CHK_STATUS_RETURN(SetPowerMode(CombinedFc));

    // Initialize command buffer and insert prolog
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnInitCommandBuffer(m_renderHal, commandBuffer, &GenericPrologParams));

    RENDER_PACKET_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectStartCmd((void*)m_renderHal, pOsInterface, pMhwMiInterface, commandBuffer));

    // Write timing data for 3P budget
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendTimingData(m_renderHal, commandBuffer, true));

    bEnableSLM = false;  // Media walker first
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSetCacheOverrideParams(
        m_renderHal,
        &m_renderHal->L3CacheSettings,
        bEnableSLM));

    // Flush media states
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendMediaStates(
        m_renderHal,
        commandBuffer,
        &m_mediaWalkerParams,
        nullptr));

    // Write back GPU Status tag
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendRcsStatusTag(m_renderHal, commandBuffer));
    }

    RENDER_PACKET_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectEndCmd((void*)m_renderHal, pOsInterface, pMhwMiInterface, commandBuffer));

    // Write timing data for 3P budget
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendTimingData(m_renderHal, commandBuffer, false));

    MHW_PIPE_CONTROL_PARAMS PipeControlParams;

    MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
    PipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    PipeControlParams.bGenericMediaStateClear = true;
    PipeControlParams.bIndirectStatePointersDisable = true;
    PipeControlParams.bDisableCSStall = false;
    RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddPipeControl(commandBuffer, nullptr, &PipeControlParams));

    if (MEDIA_IS_WA(m_renderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
    {
        MHW_VFE_PARAMS VfeStateParams = {};
        VfeStateParams.dwNumberofURBEntries = 1;
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwRender->AddMediaVfeCmd(commandBuffer, &VfeStateParams));
    }

    // Add media flush command in case HW not cleaning the media state
    if (MEDIA_IS_WA(m_renderHal->pWaTable, WaMSFWithNoWatermarkTSGHang))
    {
        FlushParam.bFlushToGo = true;
        if (m_bMediaWalker)
        {
            FlushParam.ui8InterfaceDescriptorOffset = m_mediaWalkerParams.InterfaceDescriptorOffset;
        }
        else
        {
            RENDER_PACKET_ASSERTMESSAGE("ERROR, pWalkerParams is nullptr and cannot get InterfaceDescriptorOffset.");
        }
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMediaStateFlush(commandBuffer, nullptr, &FlushParam));
    }
    else if (MEDIA_IS_WA(m_renderHal->pWaTable, WaAddMediaStateFlushCmd))
    {
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMediaStateFlush(commandBuffer, nullptr, &FlushParam));
    }

    if (pBatchBuffer)
    {
        // Send Batch Buffer end command (HW/OS dependent)
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(commandBuffer, nullptr));
    }
    else if (IsMiBBEndNeeded(pOsInterface))
    {
        // Send Batch Buffer end command for 1st level Batch Buffer
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(commandBuffer, nullptr));
    }
    else if (m_renderHal->pOsInterface->bNoParsingAssistanceInKmd)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(commandBuffer, nullptr));
    }

    // Return unused command buffer space to OS
    pOsInterface->pfnReturnCommandBuffer(pOsInterface, commandBuffer, 0);

    MOS_NULL_RENDERING_FLAGS  NullRenderingFlags;

    NullRenderingFlags =
        pOsInterface->pfnGetNullHWRenderFlags(pOsInterface);
    // Submit command buffer
    RENDER_PACKET_CHK_STATUS_RETURN(pOsInterface->pfnSubmitCommandBuffer(pOsInterface, commandBuffer, NullRenderingFlags.VPLgca || NullRenderingFlags.VPGobal));

    if ((NullRenderingFlags.VPLgca ||
        NullRenderingFlags.VPGobal) == false)
    {
        dwSyncTag = m_renderHal->pStateHeap->dwNextTag++;

        // Set media state and batch buffer as busy
        m_renderHal->pStateHeap->pCurMediaState->bBusy = true;
        if (pBatchBuffer)
        {
            pBatchBuffer->bBusy = true;
            pBatchBuffer->dwSyncTag = dwSyncTag;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::RenderEngineSetup()
{
    // pls make sure the context already switched to render/compute engine before submit
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal);

    m_bMediaWalker = m_renderHal->pfnGetMediaWalkerStatus(m_renderHal) ? true : false;

    // Register the resource of GSH
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnReset(m_renderHal));

    // Assign media state
    m_renderData.mediaState = m_renderHal->pfnAssignMediaState(m_renderHal, RENDERHAL_COMPONENT_PACKET);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderData.mediaState);

    // Allocate and reset SSH instance
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnAssignSshInstance(m_renderHal));

    // Assign and Reset binding table
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnAssignBindingTable(
        m_renderHal,
        &m_bindingTable));

    // load kernels before packet submit PipeLine to load kernel

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::InitKernelEntry()
{
    if (m_kernelCount == 0)
    {
        RENDER_PACKET_NORMALMESSAGE("no kernel set up");
        return MOS_STATUS_LOAD_LIBRARY_FAILED;
    }

    if (!m_kernelEntry)
    {
        m_kernelEntry = (Kdll_CacheEntry*)MOS_AllocAndZeroMemory(sizeof(Kdll_CacheEntry) * m_kernelCount);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::SetPowerMode(uint32_t KernelID)
{
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint16_t                            wNumRequestedEUSlices = 1;    // Default to 1 slice
    uint16_t                            wNumRequestedSubSlices = 3;    // Default to 3 subslice
    uint16_t                            wNumRequestedEUs = 8;    // Default to 8 EUs
    RENDERHAL_POWEROPTION               PowerOption;
    bool                                bSetRequestedSlices = false;
    const SseuSetting* pcSSEUTable = nullptr;

    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal);

    if ((m_renderHal->bRequestSingleSlice) || (m_renderHal->bEUSaturationNoSSD))
    {
        bSetRequestedSlices = true;
        // bEUSaturationNoSSD: No slice shutdown, must request 2 slices [CM EU saturation on].
        // bRequestSingleSlice: Always single slice.
        wNumRequestedEUSlices = (m_renderHal->bEUSaturationNoSSD) ? 2 : 1;
    }
    else
    {
        bSetRequestedSlices = false;
    }

    if (m_renderHal->sseuTable)
    {
        pcSSEUTable = (const SseuSetting*)m_renderHal->sseuTable;
    }
    else
    {
        RENDER_PACKET_ASSERTMESSAGE("SSEU Table not valid.");
        return MOS_STATUS_UNKNOWN;

    }

    RENDER_PACKET_CHK_NULL_RETURN(pcSSEUTable);
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
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_SSEU_SETTING_OVERRIDE_ID,
        &UserFeatureData);

    if (UserFeatureData.u32Data != 0xDEADC0DE)
    {
        wNumRequestedEUSlices = UserFeatureData.u32Data & 0xFF;               // Bits 0-7
        wNumRequestedSubSlices = (UserFeatureData.u32Data >> 8) & 0xFF;        // Bits 8-15
        wNumRequestedEUs = (UserFeatureData.u32Data >> 16) & 0xFFFF;     // Bits 16-31
    }
#endif

    PowerOption.nSlice = wNumRequestedEUSlices;
    PowerOption.nSubSlice = wNumRequestedSubSlices;
    PowerOption.nEU = wNumRequestedEUs;
    m_renderHal->pfnSetPowerOptionMode(m_renderHal, &PowerOption);

    return eStatus;
}

uint32_t RenderCmdPacket::SetSurfaceForHwAccess(PMOS_SURFACE surface, PRENDERHAL_SURFACE_NEXT pRenderSurface, PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams, bool bWrite)
{

    PMOS_INTERFACE                  pOsInterface;
    PRENDERHAL_SURFACE_STATE_ENTRY  pSurfaceEntries[MHW_MAX_SURFACE_PLANES];
    int32_t                         iSurfaceEntries;
    int32_t                         i;
    MOS_STATUS                      eStatus;
    RENDERHAL_SURFACE_STATE_PARAMS  surfaceParams;

    // Initialize Variables
    eStatus = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;

    RENDER_PACKET_CHK_NULL_RETURN(pRenderSurface);
    RENDER_PACKET_CHK_NULL_RETURN(pOsInterface);

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    RENDER_PACKET_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
        m_osInterface,
        &surface->OsResource,
        bWrite,
        true));

    if (!pSurfaceParams)
    {
        MOS_ZeroMemory(&surfaceParams, sizeof(RENDERHAL_SURFACE_STATE_PARAMS));
        surfaceParams.Type                  = m_renderHal->SurfaceTypeDefault;
        surfaceParams.bWidthInDword_Y       = true;
        surfaceParams.bWidthInDword_UV      = true;
        surfaceParams.Boundary              = RENDERHAL_SS_BOUNDARY_ORIGINAL;
        pSurfaceParams                      = &surfaceParams;
    }

    RENDER_PACKET_CHK_STATUS_RETURN(InitRenderHalSurface(
        *surface,
        pRenderSurface));

    if (bWrite)
    {
        pRenderSurface->SurfType = RENDERHAL_SURF_OUT_RENDERTARGET;
    }

    // Setup surface states-----------------------------------------------------
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSetupSurfaceState(
        m_renderHal,
        pRenderSurface,
        pSurfaceParams,
        &iSurfaceEntries,  // for most cases, surface entry should only take 1 entry, need align with kerenl design
        pSurfaceEntries,
        nullptr));

    if (m_bindingTableEntry > 15)
    {
        RENDER_PACKET_ASSERTMESSAGE("input surface support up to 16 RSS");
        m_bindingTableEntry = 0;
    }

    uint32_t iBTEntry = m_bindingTableEntry;
    // Bind surface states------------------------------------------------------
    for (i = 0; i < iSurfaceEntries; i++, m_bindingTableEntry++)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnBindSurfaceState(
            m_renderHal,
            m_bindingTable,
            m_bindingTableEntry,
            pSurfaceEntries[i]));

        pRenderSurface->Index = m_bindingTableEntry;
    }

    return iBTEntry;
}

uint32_t RenderCmdPacket::SetBufferForHwAccess(MOS_BUFFER buffer, PRENDERHAL_SURFACE_NEXT pRenderSurface, PRENDERHAL_SURFACE_STATE_PARAMS pSurfaceParams, bool bWrite)
{
    RENDERHAL_SURFACE                   RenderHalSurface;
    RENDERHAL_SURFACE_STATE_PARAMS      SurfaceParam;
    PRENDERHAL_SURFACE_STATE_ENTRY      pSurfaceEntry;

    RENDER_PACKET_CHK_NULL_RETURN(m_osInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_osInterface->osCpInterface);

    // not support CP yet
    if (m_osInterface->osCpInterface->IsHMEnabled())
    {
        RENDER_PACKET_ASSERTMESSAGE("ERROR, need to use VpHal_CommonSetBufferSurfaceForHwAccess if under CP HM.");
    }

    // Register surfaces for rendering (GfxAddress/Allocation index)
    // Register resource
    RENDER_PACKET_CHK_STATUS_RETURN(m_osInterface->pfnRegisterResource(
        m_osInterface,
        &buffer.OsResource,
        bWrite,
        true));

    // Setup Buffer surface-----------------------------------------------------
    if (pSurfaceParams == nullptr)
    {
        MOS_ZeroMemory(&SurfaceParam, sizeof(SurfaceParam));
        pSurfaceParams = &SurfaceParam;
    }

    RENDER_PACKET_CHK_STATUS_RETURN(InitRenderHalBuffer(
        buffer,
        &RenderHalSurface));

    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSetupBufferSurfaceState(
        m_renderHal,
        &RenderHalSurface,
        pSurfaceParams,
        &pSurfaceEntry));

    // Bind surface state-------------------------------------------------------
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnBindSurfaceState(
        m_renderHal,
        m_bindingTable,
        m_bindingTableEntry,
        pSurfaceEntry));

    pRenderSurface->Index = m_bindingTableEntry;

    m_bindingTableEntry++;
    return pRenderSurface->Index;
}

MOS_STATUS RenderCmdPacket::SetupCurbe(void* pData, uint32_t curbeLength, uint32_t maximumNumberofThreads)
{
    m_curbeOffset = m_renderHal->pfnLoadCurbeData(
        m_renderHal,
        m_renderData.mediaState,
        pData,
        curbeLength);

    if (m_curbeOffset < 0)
    {
        RENDER_PACKET_ASSERTMESSAGE("Curbe Set Fail, return error");
        return MOS_STATUS_UNKNOWN;
    }

    m_renderData.iCurbeLength += curbeLength;

    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSetVfeStateParams(
        m_renderHal,
        MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
        maximumNumberofThreads,
        m_renderData.iCurbeLength,
        m_renderData.iInlineLength,
        nullptr));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::LoadKernel()
{
    int32_t                     iKrnAllocation;
    MHW_KERNEL_PARAM            MhwKernelParam;
    RENDERHAL_KERNEL_PARAM      KernelParam;
    // Load kernel to GSH
    INIT_MHW_KERNEL_PARAM(MhwKernelParam, &m_renderData.KernelEntry);
    INIT_KERNEL_CONFIG_PARAM(KernelParam, &m_renderData.KernelParam);
    iKrnAllocation = m_renderHal->pfnLoadKernel(
        m_renderHal,
        &KernelParam,
        &MhwKernelParam,
        nullptr);

    if (iKrnAllocation < 0)
    {
        RENDER_PACKET_ASSERTMESSAGE("kernel load failed");
        return MOS_STATUS_UNKNOWN;
    }

    if (m_curbeOffset < 0)
    {
        RENDER_PACKET_ASSERTMESSAGE("Curbe Set Fail, return error");
        return MOS_STATUS_UNKNOWN;
    }
    // Allocate Media ID, link to kernel
    m_mediaID = m_renderHal->pfnAllocateMediaID(
        m_renderHal,
        iKrnAllocation,
        m_bindingTable,
        m_curbeOffset,
        (m_renderData.iCurbeLength),
        0,
        nullptr);

    if (m_mediaID < 0)
    {
        RENDER_PACKET_ASSERTMESSAGE("Allocate Media ID failed, return error");
        return MOS_STATUS_UNKNOWN;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::InitRenderHalSurface(MOS_SURFACE surface, PRENDERHAL_SURFACE pRenderSurface)
{
    RENDER_PACKET_CHK_NULL_RETURN(pRenderSurface);
    pRenderSurface->OsSurface = surface;

    RENDERHAL_GET_SURFACE_INFO info;
    MOS_ZeroMemory(&info, sizeof(info));
    RENDER_PACKET_CHK_STATUS_RETURN(RenderHal_GetSurfaceInfo(
        m_renderHal->pOsInterface,
        &info,
        &pRenderSurface->OsSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::InitRenderHalBuffer(MOS_BUFFER pSurface, PRENDERHAL_SURFACE pRenderSurface)
{
    RENDER_PACKET_CHK_NULL_RETURN(pRenderSurface);
    pRenderSurface->OsSurface.OsResource = pSurface.OsResource;

    pRenderSurface->OsSurface.OsResource         = pSurface.OsResource;
    pRenderSurface->OsSurface.dwWidth            = pSurface.size;
    pRenderSurface->OsSurface.dwHeight           = 1;
    pRenderSurface->OsSurface.dwPitch            = pSurface.size;
    pRenderSurface->OsSurface.Format             = Format_RAW;

    return MOS_STATUS_SUCCESS;
}
