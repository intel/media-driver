/*
* Copyright (c) 2009-2018, Intel Corporation
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
//! \file     vphal.cpp
//! \brief    Vphal Interface Definition
//! \details  Vphal Interface Definition Including:
//!           const and function
//!
#include "vphal.h"
#include "mos_os.h"
#include "mhw_vebox.h"
#include "renderhal.h"
#include "vphal_renderer.h"
#include "mos_solo_generic.h"
#include "media_interfaces_vphal.h"

void VphalFeatureReport::InitReportValue()
{
    IECP                =   false;
    IEF                 =   false;
    Denoise             =   false;
    ChromaDenoise       =   false;
    DeinterlaceMode     =   VPHAL_DI_REPORT_PROGRESSIVE;
    ScalingMode         =   VPHAL_SCALING_NEAREST;
    OutputPipeMode      =   VPHAL_OUTPUT_PIPE_MODE_COMP;
    VPMMCInUse          =   false;
    RTCompressible      =   false;
    RTCompressMode      =   0;
    FFDICompressible    =   false;
    FFDICompressMode    =   0;
    FFDNCompressible    =   false;
    FFDNCompressMode    =   0;
    STMMCompressible    =   false;
    STMMCompressMode    =   0;
    ScalerCompressible  =   false;
    ScalerCompressMode  =   0;
    PrimaryCompressible =   false;
    PrimaryCompressMode =   0;
    CompositionMode     =   VPHAL_NO_COMPOSITION;
}

//!
//! \brief    Allocate VPHAL Resources
//! \details  Allocate VPHAL Resources
//!           - Allocate and initialize HW states
//!           - Allocate and initialize renderer states
//! \param    [in] pVpHalSettings
//!           Pointer to VPHAL Settings
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::Allocate(
    const VphalSettings       *pVpHalSettings)
{
    MHW_VEBOX_GPUNODE_LIMIT     GpuNodeLimit;
    RENDERHAL_SETTINGS          RenderHalSettings;
    MOS_GPU_NODE                VeboxGpuNode;
    MOS_GPU_CONTEXT             VeboxGpuContext;
    MOS_STATUS                  eStatus;

    VPHAL_PUBLIC_CHK_NULL(pVpHalSettings);
    VPHAL_PUBLIC_CHK_NULL(m_renderHal);

    // Create Render GPU Context
    {
        MOS_GPUCTX_CREATOPTIONS createOption;
        VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            m_renderGpuContext,
            m_renderGpuNode,
            &createOption));
    }

    // Set current GPU context
    VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnSetGpuContext(
        m_osInterface,
        m_renderGpuContext));

    // Register Render GPU context with the event
    VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        m_renderGpuContext));

    if (MEDIA_IS_SKU(m_skuTable, FtrVERing) && m_veboxInterface)
    {
        GpuNodeLimit.bCpEnabled        = (m_osInterface->osCpInterface->IsCpEnabled())? true : false;
        GpuNodeLimit.bSfcInUse         = MEDIA_IS_SKU(m_skuTable, FtrSFCPipe);

        // Check GPU Node decide logic together in this function
        VPHAL_HW_CHK_STATUS(m_veboxInterface->FindVeboxGpuNodeToUse(&GpuNodeLimit));

        VeboxGpuNode    = (MOS_GPU_NODE)(GpuNodeLimit.dwGpuNodeToUse);
        VeboxGpuContext = (VeboxGpuNode == MOS_GPU_NODE_VE) ? MOS_GPU_CONTEXT_VEBOX : MOS_GPU_CONTEXT_VEBOX2;

        // Create VEBOX/VEBOX2 Context
        VPHAL_PUBLIC_CHK_STATUS(m_veboxInterface->CreateGpuContext(
            m_osInterface,
            VeboxGpuContext,
            VeboxGpuNode));

        // Register Vebox GPU context with the Batch Buffer completion event
        // Ignore if creation fails
        VPHAL_PUBLIC_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            MOS_GPU_CONTEXT_VEBOX));
    }

    // Allocate and initialize HW states
    RenderHalSettings.iMediaStates  = pVpHalSettings->mediaStates;
    VPHAL_PUBLIC_CHK_STATUS(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    if (m_veboxInterface &&
        m_veboxInterface->m_veboxSettings.uiNumInstances > 0 &&
        m_veboxInterface->m_veboxHeap == nullptr)
    {
        // Allocate VEBOX Heap
        VPHAL_PUBLIC_CHK_STATUS(m_veboxInterface->CreateHeap());
    }

    // Create renderer
    VPHAL_RENDER_CHK_STATUS(CreateRenderer());

    // Allocate and initialize renderer states
    VPHAL_PUBLIC_CHK_STATUS(m_renderer->Initialize(pVpHalSettings));

finish:
    return eStatus;
}

//!
//! \brief    Performs VP Rendering
//! \details  Performs VP Rendering
//!           - call default render of video
//! \param    [in] pcRenderParams
//!           Pointer to Render Params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::Render(
    PCVPHAL_RENDER_PARAMS   pcRenderParams)
{
    MOS_STATUS          eStatus;
    VPHAL_RENDER_PARAMS RenderParams;

    VPHAL_PUBLIC_CHK_NULL(pcRenderParams);

    RenderParams    = *pcRenderParams;

    // default render of video
    RenderParams.bIsDefaultStream = true;
    eStatus = m_renderer->Render((PCVPHAL_RENDER_PARAMS)(&RenderParams));

finish:
    return eStatus;
}

//!
//! \brief    Get feature reporting from renderer
//! \details  Get feature reporting from renderer
//! \return   VphalFeatureReport*
//!           Pointer to VphalFeatureReport: rendering features reported
//!
VphalFeatureReport* VphalState::GetRenderFeatureReport()
{
    VPHAL_PUBLIC_ASSERT(m_renderer);

    return m_renderer->GetReport();
}

//!
//! \brief    VphalState Constructor
//! \details  Creates instance of VphalState
//!           - Return pointer to initialized VPHAL state, but not yet fully allocated
//!           - Caller must call pfnAllocate to allocate all VPHAL states and objects.
//! \param    [in] pOsInterface
//!           OS interface, if provided externally - may be nullptr
//! \param    [in] pOsDriverContext
//!           OS driver context (UMD context, pShared, ...)
//! \param    [in,out] peStatus
//!           Pointer to the MOS_STATUS flag.
//!           Will assign this flag to MOS_STATUS_SUCCESS if successful, otherwise failed
//!
VphalState::VphalState(
        PMOS_INTERFACE          pOsInterface,
        PMOS_CONTEXT            pOsDriverContext,
        MOS_STATUS              *peStatus) :
        m_platform(),
        m_skuTable(nullptr),
        m_waTable(nullptr),
        m_osInterface(pOsInterface),
        m_renderHal(nullptr),
        m_cpInterface(nullptr),
        m_sfcInterface(nullptr),
        m_renderer(nullptr),
        m_veboxInterface(nullptr),
        m_renderGpuNode(MOS_GPU_NODE_3D),
        m_renderGpuContext(MOS_GPU_CONTEXT_RENDER)
{
    MOS_STATUS                  eStatus;

    eStatus                     = MOS_STATUS_UNKNOWN;

    // Initialize platform, sku, wa tables
    m_osInterface->pfnGetPlatform(m_osInterface, &m_platform);
    m_skuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    m_waTable  = m_osInterface->pfnGetWaTable (m_osInterface);

    m_renderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(*m_renderHal));
    VPHAL_PUBLIC_CHK_NULL(m_renderHal);
    VPHAL_PUBLIC_CHK_STATUS(RenderHal_InitInterface(
        m_renderHal,
        &m_cpInterface,
        m_osInterface));

finish:
    if(peStatus)
    {
        *peStatus = eStatus;
    }
}

//!
//! \brief    Vphal Destructor
//! \details  Destroys VPHAL and all internal states and objects
//! \return   void
//!
VphalState::~VphalState()
{
    MOS_STATUS              eStatus;

    // Destroy rendering objects (intermediate surfaces, BBs, etc)
    if (m_renderer)
    {
        MOS_Delete(m_renderer);
        m_renderer = nullptr;
    }

    if (m_renderHal)
    {
        eStatus = m_renderHal->pfnDestroy(m_renderHal);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
        }
        MOS_FreeMemory(m_renderHal);
    }

    if (m_cpInterface)
    {
        MOS_Delete(m_cpInterface);
    }

    if (m_sfcInterface)
    {
        MOS_Delete(m_sfcInterface);
        m_sfcInterface = nullptr;
    }

    if (m_veboxInterface)
    {
        eStatus = m_veboxInterface->DestroyHeap();
        MOS_Delete(m_veboxInterface);
        m_veboxInterface = nullptr;
        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_PUBLIC_ASSERTMESSAGE("Failed to destroy Vebox Interface, eStatus:%d.\n", eStatus);
        }
    }

    // Destroy OS interface objects (CBs, etc)
    if (m_osInterface)
    {
        if (m_osInterface->bDeallocateOnExit)
        {
            m_osInterface->pfnDestroy(m_osInterface, true);

            // Deallocate OS interface structure (except if externally provided)
            MOS_FreeMemory(m_osInterface);
        }
    }
}

VphalState* VphalState::VphalStateFactory(
    PMOS_INTERFACE          pOsInterface,
    PMOS_CONTEXT            pOsDriverContext,
    MOS_STATUS              *peStatus)
{
    return VphalDevice::CreateFactory(pOsInterface, pOsDriverContext, peStatus);
}

//!
//! \brief    Get Status Report
//! \details  Get Status Report, will return back to app indicating if related frame id is done by gpu
//! \param    [out] pQueryReport
//!           Pointer to pQueryReport, the status query report array.
//! \param    [in] wStatusNum
//!           The size of array pQueryReport.
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
MOS_STATUS VphalState::GetStatusReport(
    PQUERY_STATUS_REPORT_APP       pQueryReport,
    uint16_t                       wStatusNum)
{
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;

#if (!EMUL && !ANDROID)        // this function is dummy for android and emul
    uint32_t                       i;
    uint32_t                       uiTableLen;
    PVPHAL_STATUS_TABLE            pStatusTable;
    PMOS_CONTEXT                   pOsContext;
    uint32_t                       uiIndex;
    uint32_t                       uiNewHead;
    PVPHAL_STATUS_ENTRY            pStatusEntry;
    bool                           bMarkNotReadyForRemains = false;

    VPHAL_PUBLIC_CHK_NULL(pQueryReport);
    VPHAL_PUBLIC_CHK_NULL(m_osInterface);
    VPHAL_PUBLIC_CHK_NULL(m_osInterface->pOsContext);

    // it should be ok if we don't consider the null render
    // eNullRender = m_pOsInterface->pfnGetNullHWRenderFlags(m_pOsInterface);

    pOsContext           = m_osInterface->pOsContext;
    pStatusTable         = &m_renderer->StatusTable;
    uiNewHead            = pStatusTable->uiHead; // uiNewHead start from previous head value
    // entry length from head to tail
    uiTableLen           = (pStatusTable->uiCurrent - pStatusTable->uiHead) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);

    // step 1 - update pStatusEntry from driver if command associated with the dwTag is done by gpu
    for (i = 0; i < wStatusNum && i < uiTableLen; i++)
    {
        uint32_t    dwGpuTag; // hardware tag updated by gpu command pipectl
        bool        bDoneByGpu;
        bool        bFailedOnSubmitCmd;

        uiIndex            = (pStatusTable->uiHead + i) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        pStatusEntry       = &pStatusTable->aTableEntries[uiIndex];

        if (bMarkNotReadyForRemains)
        {
            // the status is set as VPREP_NOTREADY while submitting commands
            pQueryReport[i].dwStatus         = pStatusEntry->dwStatus;
            pQueryReport[i].StatusFeedBackID = pStatusEntry->StatusFeedBackID;
            continue;
        }

#if LINUX
        dwGpuTag           = pOsContext->GetGPUTag(m_osInterface, pStatusEntry->GpuContextOrdinal);
#else
        dwGpuTag           = pOsContext->GetGPUTag(pOsContext->GetGpuContextHandle(pStatusEntry->GpuContextOrdinal));
#endif
        bDoneByGpu         = (dwGpuTag >= pStatusEntry->dwTag);
        bFailedOnSubmitCmd = (pStatusEntry->dwStatus == VPREP_ERROR);

#if (_DEBUG || _RELEASE_INTERNAL)
        MOS_NULL_RENDERING_FLAGS NullRender = m_osInterface->pfnGetNullHWRenderFlags(m_osInterface);
        if (NullRender.Value != 0)
        {
            bDoneByGpu = true;
        }
#endif

        if (bFailedOnSubmitCmd)
        {
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        }
        else if (bDoneByGpu)
        {
            pStatusEntry->dwStatus = VPREP_OK;
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        }
        else
        {   // here we have the first not ready entry.
#if LINUX
            uiNewHead = (uiIndex + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
#else
            uiNewHead = uiIndex;
#endif
            bMarkNotReadyForRemains = true;
        }

        if (m_osInterface->pfnIsGPUHung(m_osInterface))
        {
            pStatusEntry->dwStatus = VPREP_NOTREADY;
        }

        pQueryReport[i].dwStatus         = pStatusEntry->dwStatus;
        pQueryReport[i].StatusFeedBackID = pStatusEntry->StatusFeedBackID;
    }
    pStatusTable->uiHead = uiNewHead;

    // step 2 - mark VPREP_NOTAVAILABLE for unused entry
    for (/* continue from previous i */; i < wStatusNum; i++)
    {
        pQueryReport[i].dwStatus         = VPREP_NOTAVAILABLE;
        pQueryReport[i].StatusFeedBackID = 0;
    }

finish:
#else
    MOS_UNUSED(pQueryReport);
    MOS_UNUSED(wStatusNum);
#endif // end (!EMUL && !ANDROID)
    return eStatus;
}

//!
//! \brief    Get Status Report's entry length from head to tail
//! \details  Get Status Report's entry length from head to tail
//! \param    [out] puiLength
//!           Pointer to the entry length
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VphalState::GetStatusReportEntryLength(
    uint32_t*                      puiLength)
{
    MOS_STATUS                     eStatus = MOS_STATUS_SUCCESS;
#if __linux__  // this function is only for Linux now
    PVPHAL_STATUS_TABLE            pStatusTable;

    VPHAL_PUBLIC_CHK_NULL(m_renderer);
    VPHAL_PUBLIC_CHK_NULL(puiLength);

    pStatusTable = &m_renderer->StatusTable;

    // entry length from head to tail
    *puiLength = (pStatusTable->uiCurrent - pStatusTable->uiHead) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
finish:
#else
    MOS_UNUSED(puiLength);
#endif
    return eStatus;
}
