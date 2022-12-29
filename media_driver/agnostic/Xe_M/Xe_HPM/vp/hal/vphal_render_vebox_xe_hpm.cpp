/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     vphal_render_vebox_xe_hpm.cpp
//! \brief    Interface and structure specific for Xe_HPM Vebox
//! \details  Interface and structure specific for Xe_HPM Vebox
//!
#include "vphal.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_vebox_xe_hpm.h"
#include "mhw_vebox_xe_hpm.h"
#include "mos_interface.h"
#include "vphal_debug.h"
#include "vp_hal_ddi_utils.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_isa_xe_hpg.h"
#endif

VPHAL_VEBOX_STATE_XE_HPM::VPHAL_VEBOX_STATE_XE_HPM(
    PMOS_INTERFACE                 pOsInterface,
    PMHW_VEBOX_INTERFACE           pVeboxInterface,
    PMHW_SFC_INTERFACE             pSfcInterface,
    PRENDERHAL_INTERFACE           pRenderHal,
    PVPHAL_VEBOX_EXEC_STATE        pVeboxExecState,
    PVPHAL_RNDR_PERF_DATA          pPerfData,
    const VPHAL_DNDI_CACHE_CNTL    &dndiCacheCntl,
    MOS_STATUS                     *peStatus) :
    VPHAL_VEBOX_STATE(pOsInterface, pVeboxInterface, pSfcInterface, pRenderHal, pVeboxExecState, pPerfData, dndiCacheCntl, peStatus),
    VPHAL_VEBOX_STATE_G12_BASE(pOsInterface, pVeboxInterface, pSfcInterface, pRenderHal, pVeboxExecState, pPerfData, dndiCacheCntl, peStatus),
    VPHAL_VEBOX_STATE_XE_XPM(pOsInterface, pVeboxInterface, pSfcInterface, pRenderHal, pVeboxExecState, pPerfData, dndiCacheCntl, peStatus)
{
    uint32_t i;
    uint32_t            veboxMaxPipeNum = 0;
    MEDIA_SYSTEM_INFO   *gtSystemInfo    = nullptr;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_hdr3DLutKernelBinary     = (uint32_t *)IGVP3DLUT_GENERATION_XE_HPG;
    m_hdr3DLutKernelBinarySize = IGVP3DLUT_GENERATION_XE_HPG_SIZE;
#endif

    // Vebox Scalability
    bVeboxScalableMode = false;  //!< Vebox Scalable Mode
    if(!pOsInterface)
    {
        *peStatus = MOS_STATUS_NULL_POINTER;
        return;
    }

    gtSystemInfo = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
    if (gtSystemInfo)
    {
        veboxMaxPipeNum = gtSystemInfo->MaxVECS;
    }

    for (i = 0; i < veboxMaxPipeNum; i++)
    {
        PMOS_COMMAND_BUFFER pCmdBuffer = (PMOS_COMMAND_BUFFER)MOS_AllocAndZeroMemory(sizeof(MOS_COMMAND_BUFFER));
        if (pCmdBuffer == nullptr)
        {
            *peStatus = MOS_STATUS_NO_SPACE;
            return;
        }
        m_veCmdBuffers.emplace_back(pCmdBuffer);
    }

    dwVECmdBufSize = 0;  //!< Command Buffer Size
    for (i = 0; i < MHW_VEBOX_MAX_SEMAPHORE_NUM_G12; i++)
    {
        VESemaMemS[i] = {};
    }
    dwNumofVebox = 0;

#if LINUX
    char* ScalingHQPerfMode = getenv("SET_SCALINGHQ_AS_PERFMODE");
    if (ScalingHQPerfMode)
    {
        bScalingHQPefMode = strcmp(ScalingHQPerfMode, "ON")?false:true;
    }
#endif
}

VPHAL_VEBOX_STATE_XE_HPM::~VPHAL_VEBOX_STATE_XE_HPM()
{
    for (auto &icmdBuffer : m_veCmdBuffers)
    {
        if (icmdBuffer)
        {
            MOS_FreeMemory(icmdBuffer);
        }
        icmdBuffer = nullptr;
    }
    m_veCmdBuffers.clear();
    return;
}

//!
//! \brief    Vebox allocate resources
//! \details  Allocate resources that will be used in Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_HPM::AllocateResources()
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_STATE_XE_HPM   pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA     pRenderData;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pVeboxInterface);

    pRenderData     = GetLastExecRenderData();

    VPHAL_RENDER_CHK_STATUS(VPHAL_VEBOX_STATE_XE_XPM::AllocateResources());

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        pVeboxState->FreeResources();
    }

    return eStatus;
}

//!
//! \brief    Vebox state heap update for Auto-DN/ACE
//! \details  update Vebox states (DN, ACE),
//!           in normal case, use CPU to update the Vebox state heap in clear memory (DriverResource).
//!           Otherwise use kernel to update the Vebox state heap in non-clear memory (KernelResource)
//! \param    PVPHAL_SURFACE pSrcSurface
//!           [in] Pointer to input surface of Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_HPM::VeboxUpdateVeboxStates(
    PVPHAL_SURFACE pSrcSurface)
{
    PRENDERHAL_INTERFACE          pRenderHal;
    PMOS_INTERFACE                pOsInterface;
    MOS_STATUS                    eStatus;
    bool                          bCPUUpdate;
    uint8_t *                     pStat;
    uint8_t *                     pStatSlice0Base, *pStatSlice1Base;
    int32_t                       iCurbeOffsetDN;
    int32_t                       iKrnAllocation;
    MHW_KERNEL_PARAM              MhwKernelParam;
    uint32_t                      dwKernelUpdate;
    uint32_t                      dwQuery;
    MOS_LOCK_PARAMS               LockFlags;
    PVPHAL_VEBOX_STATE_XE_HPM     pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(pSrcSurface);

    eStatus             = MOS_STATUS_SUCCESS;
    pRenderHal          = pVeboxState->m_pRenderHal;
    pOsInterface        = pVeboxState->m_pOsInterface;
    dwKernelUpdate      = pVeboxState->dwKernelUpdate;

#if VEBOX_AUTO_DENOISE_SUPPORTED
    if (!(pRenderData->bAutoDenoise ||
       (pRenderData->bDenoise) ||
       (pRenderData->bDenoise && !bFirstFrame && m_bTgneEnable)))
    {
        // no need to update, direct return.
        VPHAL_RENDER_NORMALMESSAGE("No need update vebox states.");
        return MOS_STATUS_SUCCESS;
    }

    VPHAL_RENDER_NORMALMESSAGE("Update vebox states.");

    // Update DN State in CPU
    bCPUUpdate = !(dwKernelUpdate);

    if (bCPUUpdate)
    {
        MOS_ZeroMemory(&LockFlags, sizeof(MOS_LOCK_PARAMS));
        LockFlags.ReadOnly = 1;

        // Get Statistic surface
        pStat = (uint8_t *)pOsInterface->pfnLockResource(
            pOsInterface,
            &pVeboxState->VeboxStatisticsSurface.OsResource,
            &LockFlags);

        VPHAL_RENDER_CHK_NULL(pStat);

        VPHAL_RENDER_CHK_STATUS(VeboxGetStatisticsSurfaceBase(
            pStat,
            &pStatSlice0Base,
            &pStatSlice1Base));

        VPHAL_DBG_STATE_DUMPPER_DUMP_VEBOX_STATISTICS(pRenderHal, (void *)pVeboxState, pStatSlice0Base, pStatSlice1Base);

        // Get GNE from Statistics
        if (pRenderData->bDenoise)
        {
            // Query platform dependent GNE offset
            VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxQueryStatLayout(
                VEBOX_STAT_QUERY_GNE_OFFEST,
                &dwQuery));

            // check TGNE is valid or not.
            VPHAL_RENDER_CHK_STATUS(pVeboxState->CheckTGNEValid(
                (uint32_t *)(pStatSlice0Base + dwQuery),
                (uint32_t *)(pStatSlice1Base + dwQuery),
                &dwQuery));

            if (pSrcSurface->pDenoiseParams->bEnableHVSDenoise)
            {
                VPHAL_RENDER_CHK_STATUS(VeboxUpdateDnStatesForHVS(
                    pSrcSurface->pDenoiseParams,
                    (uint32_t *)(pStatSlice0Base + dwQuery),
                    (uint32_t *)(pStatSlice1Base + dwQuery)));
            }
        }

        // unlock the statistic surface
        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnUnlockResource(
            pOsInterface,
            &pVeboxState->VeboxStatisticsSurface.OsResource));

        // Set up the Vebox State in Clear Memory
        VPHAL_RENDER_CHK_STATUS(VeboxUpdateVeboxHwStates(
            pSrcSurface,
            pRenderData->GetVeboxStateParams()));
    }
    else  // launch update kernel to update VEBOX state
    {
        // Switch GPU Context to Render Engine
        pOsInterface->pfnSetGpuContext(pOsInterface, RenderGpuContext);

        // Reset allocation list and house keeping
        pOsInterface->pfnResetOsStates(pOsInterface);

        // Register the resource of GSH
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnReset(pRenderHal));

        // Set up UpdateDNState kernel
        if (pRenderData->bAutoDenoise)
        {
            pVeboxState->SetupVeboxKernel(KERNEL_UPDATEDNSTATE);
        }

        //----------------------------------
        // Allocate and reset media state
        //----------------------------------
        pRenderData->pMediaState = pRenderHal->pfnAssignMediaState(pRenderHal, RENDERHAL_COMPONENT_VEBOX);
        VPHAL_RENDER_CHK_NULL(pRenderData->pMediaState);

        //----------------------------------
        //Allocate and reset SSH instance
        //----------------------------------
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignSshInstance(pRenderHal));

        //----------------------------------
        // Assign and Reset Binding Table
        //----------------------------------
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnAssignBindingTable(
            pRenderHal,
            &pRenderData->iBindingTable));

        //------------------------------------------
        // Setup Surface states for DN Update kernel
        //------------------------------------------
        if (pRenderData->bAutoDenoise)
        {
            VPHAL_RENDER_CHK_STATUS(pVeboxState->SetupSurfaceStatesForDenoise());
        }

        //----------------------------------
        // Load static data (platform specific)
        //----------------------------------
        VPHAL_RENDER_CHK_STATUS(pVeboxState->LoadUpdateDenoiseKernelStaticData(
            &iCurbeOffsetDN));

        //----------------------------------
        // Setup VFE State params. Each Renderer MUST call pfnSetVfeStateParams().
        //----------------------------------
        VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnSetVfeStateParams(
            pRenderHal,
            MEDIASTATE_DEBUG_COUNTER_FREE_RUNNING,
            pVeboxState->pKernelParamTable[KERNEL_UPDATEDNSTATE].Thread_Count,
            pRenderData->iCurbeLength,
            pRenderData->iInlineLength,
            nullptr));

        //----------------------------------
        // Load DN update kernel to GSH
        //----------------------------------
        if (pRenderData->bAutoDenoise)
        {
            INIT_MHW_KERNEL_PARAM(MhwKernelParam, &pRenderData->KernelEntry[KERNEL_UPDATEDNSTATE]);
            iKrnAllocation = pRenderHal->pfnLoadKernel(
                pRenderHal,
                pRenderData->pKernelParam[KERNEL_UPDATEDNSTATE],
                &MhwKernelParam,
                nullptr);

            if (iKrnAllocation < 0)
            {
                eStatus = MOS_STATUS_UNKNOWN;
                goto finish;
            }

            //----------------------------------
            // Allocate Media ID, link to kernel
            //----------------------------------
            pRenderData->iMediaID0 = pRenderHal->pfnAllocateMediaID(
                pRenderHal,
                iKrnAllocation,
                pRenderData->iBindingTable,
                iCurbeOffsetDN,
                pRenderData->pKernelParam[KERNEL_UPDATEDNSTATE]->CURBE_Length << 5,
                0,
                nullptr);

            if (pRenderData->iMediaID0 < 0)
            {
                eStatus = MOS_STATUS_UNKNOWN;
                goto finish;
            }
        }

        //---------------------------
        // Render Batch Buffer (Submit commands to HW)
        //---------------------------
        VPHAL_RENDER_CHK_STATUS(VeboxFlushUpdateStateCmdBuffer());
    }

finish:
    return eStatus;
#else
finish:
    return MOS_STATUS_SUCCESS;
#endif
}

MOS_STATUS VPHAL_VEBOX_STATE_XE_HPM::CheckTGNEValid(
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr,
    uint32_t *pQuery)
{
    uint32_t                      bGNECountLumaValid = 0;
    PVPHAL_VEBOX_STATE_XE_HPM     pVeboxState        = this;
    uint32_t                      dwTGNEoffset       = 0;
    MOS_STATUS                    eStatus;

    VPHAL_RENDER_CHK_NULL(pVeboxState);

    eStatus      = MOS_STATUS_SUCCESS;
    dwTGNEoffset = (VPHAL_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET_G12 - VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12) / 4;

    if (m_bTgneEnable)
    {
        bGNECountLumaValid = (pStatSlice0GNEPtr[dwTGNEoffset + 3] & 0x80000000) || (pStatSlice1GNEPtr[dwTGNEoffset + 3] & 0x80000000);

        VPHAL_RENDER_NORMALMESSAGE("TGNE:bGNECountLumaValid %x", bGNECountLumaValid);

        if (bGNECountLumaValid)
        {
            *pQuery     = VPHAL_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET_G12;
            bTGNE_Valid = true;

            if (bTGNE_FirstFrame)
            {
                bTGNE_FirstFrame = false;
            }
        }
        else
        {
            *pQuery     = VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12;
            bTGNE_Valid = false;
        }
    }
    else
    {
        *pQuery          = VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12;
        bTGNE_Valid      = false;
        bTGNE_FirstFrame = true;
    }

finish:
    VPHAL_RENDER_NORMALMESSAGE("TGNE:bTGNEValid %x", bTGNE_Valid);
    return eStatus;
}

//!
//! \brief    Consistent Check the value of GNE Luma
//! \details  Consistent Check the value of GNE Luma
//! \param    [in, out] dwGNELuma
//!           Spatial GNE in Y channel
//! \param    uint32_t* pStatSlice0GNEPtr
//!           [in] Pointer to Vebox slice0 GNE data
//! \param    uint32_t* pStatSlice1GNEPtr
//!           [in] Pointer to Vebox slice1 GNE data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_HPM::GNELumaConsistentCheck(
    uint32_t &dwGNELuma,
    uint32_t *pStatSlice0GNEPtr,
    uint32_t *pStatSlice1GNEPtr)
{
    MOS_STATUS eStatus;
    uint32_t   dwGNEChromaU, dwGNEChromaV;
    uint32_t   dwGNECountChromaU, dwGNECountChromaV;
    VPHAL_RENDER_CHK_NULL(pStatSlice0GNEPtr);
    VPHAL_RENDER_CHK_NULL(pStatSlice1GNEPtr);

    eStatus           = MOS_STATUS_SUCCESS;
    dwGNEChromaU      = 0;
    dwGNEChromaV      = 0;
    dwGNECountChromaU = 0;
    dwGNECountChromaV = 0;

    // Combine the GNE in slice0 and slice1 to generate the global GNE and Count
    dwGNEChromaU = pStatSlice0GNEPtr[1] + pStatSlice1GNEPtr[1];
    dwGNEChromaV = pStatSlice0GNEPtr[2] + pStatSlice1GNEPtr[2];

    dwGNECountChromaU = pStatSlice0GNEPtr[4] + pStatSlice1GNEPtr[4];
    dwGNECountChromaV = pStatSlice0GNEPtr[5] + pStatSlice1GNEPtr[5];

    // Validate GNE
    if (dwGNEChromaU == 0xFFFFFFFF || dwGNECountChromaU == 0xFFFFFFFF ||
        dwGNEChromaV == 0xFFFFFFFF || dwGNECountChromaV == 0xFFFFFFFF)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
    dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);
    VPHAL_RENDER_NORMALMESSAGE("dwGNEChromaU %d  dwGNEChromaV %d", dwGNEChromaU, dwGNEChromaV);
    if ((dwGNEChromaU < NOSIE_GNE_CHROMA_THRESHOLD) &&
        (dwGNEChromaV < NOSIE_GNE_CHROMA_THRESHOLD) &&
        (dwGNEChromaU != 0) &&
        (dwGNEChromaV != 0) &&
        (dwGNELuma > NOSIE_GNE_LUMA_THRESHOLD))
    {
        dwGNELuma = dwGNELuma >> 2;
    }

finish:
    return eStatus;
}

//!
//! \brief    Vebox HW state heap update for Auto-DN
//! \details  update Vebox HW states (DN),
//! \param    PVPHAL_SURFACE pSrcSurface
//!           [in] Pointer to input surface of Vebox
//! \param    PVPHAL_VEBOX_PARAMS_EXT pVeboxStateParams
//!           [in] Pointer to VEBOX State Params
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_HPM::VeboxUpdateVeboxHwStates(
    PVPHAL_SURFACE                pSrcSurface,
    PVPHAL_VEBOX_STATE_PARAMS     pVeboxStateParams)
{
    PRENDERHAL_INTERFACE         pRenderHal = nullptr;
    PMHW_VEBOX_INTERFACE         pVeboxInterface = nullptr;
    MHW_VEBOX_IECP_PARAMS        VeboxIecpParams;
    MOS_STATUS                   eStatus;
    PVPHAL_VEBOX_STATE_XE_HPM    pVeboxState              = this;
    PVPHAL_VEBOX_RENDER_DATA     pRenderData              = GetLastExecRenderData();

    pRenderHal      = pVeboxState->m_pRenderHal;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    if (!pVeboxStateParams->pVphalVeboxDndiParams)
    {
        eStatus = MOS_STATUS_UNKNOWN;
        goto finish;
    }

    eStatus = MOS_STATUS_SUCCESS;
    
    if (pVeboxStateParams->pVphalVeboxDndiParams)
    {
        MhwVeboxInterfaceG12 *pVeboxInterface12;
        pVeboxInterface12 = static_cast<MhwVeboxInterfaceG12 *>(pVeboxInterface);

        VPHAL_RENDER_CHK_STATUS(pVeboxInterface12->SetVeboxChromaParams(
            (MhwVeboxInterfaceG12::MHW_VEBOX_CHROMA_PARAMS *)&pRenderData->VeboxChromaParams));

        VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxDndiState(
            pVeboxStateParams->pVphalVeboxDndiParams));
    }

finish:
    return eStatus;
}

//!
//! \brief    Vebox update HVS DN states
//! \details  CPU update for VEBOX HVS DN states
//! \param    PVPHAL_DENOISE_PARAMS pDNParams
//!           [in] Pointer to DN parameter
//! \param    uint32_t* pStatSlice0GNEPtr
//!           [out] Pointer to Vebox slice0 GNE data
//! \param    uint32_t* pStatSlice1GNEPtr
//!           [out] Pointer to Vebox slice1 GNE data
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_HPM::VeboxUpdateDnStatesForHVS(
    PVPHAL_DENOISE_PARAMS pDNParams,
    uint32_t *            pStatSlice0GNEPtr,
    uint32_t *            pStatSlice1GNEPtr)
{
    MOS_STATUS                eStatus;
    uint32_t                  dwGNELuma, dwGNEChromaU, dwGNEChromaV;
    uint32_t                  dwGNECountLuma, dwGNECountChromaU, dwGNECountChromaV;
    PVPHAL_DNUV_PARAMS        pChromaParams = nullptr;
    VPHAL_DNUV_PARAMS         ChromaParams;
    PMHW_VEBOX_DNDI_PARAMS    pDNDIParams;
    PVPHAL_VEBOX_STATE_XE_HPM pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA  pRenderData = GetLastExecRenderData();
    PMHW_VEBOX_INTERFACE      pVeboxInterface;
    MhwVeboxInterfaceXe_Hpm * pVeboxInterfaceXe_Hpm;
    int32_t                   sgne_offset     = 0;
    uint32_t                  dwSgneCountLuma = 0, dwSgneCountU = 0, dwSgneCountV;
    uint32_t                  dwSgneLuma = 0, dwSgneU = 0, dwSgneV = 0;
    uint32_t                  resltn = 0;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pVeboxInterface);
    VPHAL_RENDER_CHK_NULL(pRenderData);
    VPHAL_RENDER_CHK_NULL(pDNParams);

    eStatus           = MOS_STATUS_SUCCESS;
    dwGNELuma         = 0;
    dwGNEChromaU      = 0;
    dwGNEChromaV      = 0;
    dwGNECountLuma    = 0;
    dwGNECountChromaU = 0;
    dwGNECountChromaV = 0;
    pChromaParams     = nullptr;
    pDNDIParams       = pRenderData->GetVeboxStateParams()->pVphalVeboxDndiParams;
    resltn            = pVeboxState->m_currentSurface->dwWidth * pVeboxState->m_currentSurface->dwHeight;

    VPHAL_RENDER_CHK_NULL(pDNDIParams);

    // update the DNDI parameters
    PMHW_VEBOX_DNDI_PARAMS pVeboxDNDIParams;
    // Populate the VEBOX VEBOX parameters
    pVeboxDNDIParams = &pRenderData->VeboxDNDIParams;
    // Pick up already programmed states from clear memory, since we perform a complete refresh of VEBOX params
    pVeboxDNDIParams->bDNDITopFirst             = pDNDIParams->bDNDITopFirst;
    pVeboxDNDIParams->bProgressiveDN            = pDNDIParams->bProgressiveDN;
    pVeboxDNDIParams->dwFMDFirstFieldCurrFrame  = pDNDIParams->dwFMDFirstFieldCurrFrame;
    pVeboxDNDIParams->dwFMDSecondFieldPrevFrame = pDNDIParams->dwFMDSecondFieldPrevFrame;

    // Only need to reverse bDNDITopFirst for no reference case, no need to reverse it for having refrenece case
    if (!pRenderData->bRefValid)
    {
        pVeboxDNDIParams->bDNDITopFirst = pRenderData->bTopField;
    }

    // Combine the GNE in slice0 and slice1 to generate the global GNE and Count
    dwGNELuma = pStatSlice0GNEPtr[0] + pStatSlice1GNEPtr[0];

    pVeboxInterface       = pVeboxState->m_pVeboxInterface;
    pVeboxInterfaceXe_Hpm = (MhwVeboxInterfaceXe_Hpm *)pVeboxInterface;

    VPHAL_RENDER_NORMALMESSAGE("m_bTgneEnable %d, bTGNE_Valid %d, bTGNE_FirstFrame %d, bFirstFrame %d",
        m_bTgneEnable,
        bTGNE_Valid,
        bTGNE_FirstFrame,
        bFirstFrame);

    // Set HVS kernel Params
    pDNParams->HVSDenoise.Fallback          = !bTGNE_Valid && !bFirstFrame && !bTGNE_FirstFrame;
    pDNParams->HVSDenoise.EnableChroma      = pRenderData->bChromaDenoise;
    pDNParams->HVSDenoise.TgneEnable        = m_bTgneEnable;
    pDNParams->HVSDenoise.FirstFrame        = bFirstFrame;
    pDNParams->HVSDenoise.TgneFirstFrame    = !bFirstFrame && bTGNE_FirstFrame;
    pDNParams->HVSDenoise.EnableTemporalGNE = m_bTgneEnable;
    pDNParams->HVSDenoise.Width             = (uint16_t)pVeboxState->m_currentSurface->dwWidth;
    pDNParams->HVSDenoise.Height            = (uint16_t)pVeboxState->m_currentSurface->dwHeight;

    if (pDNParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE && pDNParams->bEnableHVSDenoise)
    {
        pVeboxInterfaceXe_Hpm->bHVSAutoBdrateEnable = true;
    }
    else if (pDNParams->HVSDenoise.Mode == HVSDENOISE_AUTO_SUBJECTIVE && pDNParams->bEnableHVSDenoise)
    {
        pVeboxInterfaceXe_Hpm->bHVSAutoSubjectiveEnable = true;
        pVeboxInterfaceXe_Hpm->dwBSDThreshold           = (resltn < RESOLUTION_THRESHOLD) ? 240 : 135;
    }
    else
    {
        pVeboxInterfaceXe_Hpm->bHVSAutoBdrateEnable     = false;
        pVeboxInterfaceXe_Hpm->bHVSAutoSubjectiveEnable = false;
    }

    if (m_bTgneEnable && bTGNE_FirstFrame && !bFirstFrame)  //2nd frame
    {
        pVeboxInterfaceXe_Hpm->bTGNEEnable  = true;
        pVeboxInterfaceXe_Hpm->dwLumaStadTh = 3200;
        if (MEDIA_IS_WA(pVeboxState->m_pRenderHal->pWaTable, Wa_1609102037) &&
            VpHalDDIUtils::GetSurfaceColorPack(pVeboxState->m_currentSurface->Format) == VPHAL_COLORPACK_444)
        {
            pVeboxInterfaceXe_Hpm->dw4X4TGNEThCnt = ((pVeboxState->m_currentSurface->dwWidth - 32) *
                                                        (pVeboxState->m_currentSurface->dwHeight - 8)) /
                                                    1600;
        }
        else
        {
            pVeboxInterfaceXe_Hpm->dw4X4TGNEThCnt = ((pVeboxState->m_currentSurface->dwWidth - 8) *
                                                        (pVeboxState->m_currentSurface->dwHeight - 8)) /
                                                    1600;
        }

        if (pDNParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE)
        {
            pVeboxInterfaceXe_Hpm->dwLumaStadTh  = 250;
            pVeboxInterfaceXe_Hpm->dwHistoryInit = 27;
            dwGNECountLuma                       = pStatSlice0GNEPtr[3] + pStatSlice1GNEPtr[3];
            dwGNELuma                            = dwGNELuma * 100 / (dwGNECountLuma + 1);
            // Validate GNE
            if (dwGNELuma == 0xFFFFFFFF || dwGNECountLuma == 0xFFFFFFFF)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
                eStatus = MOS_STATUS_INVALID_PARAMETER;
                goto finish;
            }

            // consistent check
            if (pRenderData->bChromaDenoise)
            {
                GNELumaConsistentCheck(dwGNELuma, pStatSlice0GNEPtr, pStatSlice1GNEPtr);
            }
            dwGlobalNoiseLevel_Temporal = (dwGNELuma + 50) / 100;
        }

        pDNParams->HVSDenoise.PrevNslvTemporal   = -1;
        pDNParams->HVSDenoise.Sgne_Level         = dwGNELuma;
        pDNParams->HVSDenoise.Sgne_Count         = dwGNECountLuma;
        pDNParams->HVSDenoise.dwGlobalNoiseLevel = dwGlobalNoiseLevel_Temporal;
    }
    else if (m_bTgneEnable && bTGNE_Valid && !bFirstFrame)  // middle frame
    {
        dwGNECountLuma = (pStatSlice0GNEPtr[3] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[3] & 0x7FFFFFFF);

        // Validate TGNE
        if (dwGNECountLuma == 0)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Incorrect GNE count.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        VPHAL_RENDER_NORMALMESSAGE("GNELuma 0x%x  GNECountLuma 0x%x", dwGNELuma, dwGNECountLuma);
        curNoiseLevel_Temporal      = dwGNELuma / dwGNECountLuma;
        dwGlobalNoiseLevel_Temporal = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevel_Temporal + curNoiseLevel_Temporal, 1);

        if (pVeboxInterfaceXe_Hpm->bHVSfallback)
        {
            pDNParams->HVSDenoise.dwGlobalNoiseLevel = curNoiseLevel_Temporal;
        }
        else
        {
            pDNParams->HVSDenoise.dwGlobalNoiseLevel = dwGlobalNoiseLevel_Temporal;
        }
        pVeboxInterfaceXe_Hpm->bTGNEEnable     = true;
        sgne_offset                            = -12;  //VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12 - VPHAL_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET_G12;
        dwSgneCountLuma                        = pStatSlice0GNEPtr[sgne_offset + 3] + pStatSlice1GNEPtr[sgne_offset + 3];
        dwSgneLuma                             = pStatSlice0GNEPtr[sgne_offset] + pStatSlice1GNEPtr[sgne_offset];
        pDNParams->HVSDenoise.Sgne_Level       = dwSgneLuma * 100 / (dwSgneCountLuma + 1);
        pDNParams->HVSDenoise.Sgne_Count       = dwSgneCountLuma;
        pDNParams->HVSDenoise.PrevNslvTemporal = curNoiseLevel_Temporal;

        if (pDNParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE)
        {
            pVeboxInterfaceXe_Hpm->dwLumaStadTh  = 250;
            pVeboxInterfaceXe_Hpm->dwHistoryInit = 27;
        }
        else
        {
            pVeboxInterfaceXe_Hpm->dwLumaStadTh = (dwGlobalNoiseLevel_Temporal <= 1) ? 3200 : (pVeboxInterfaceXe_Hpm->dwLumaStadTh + (curNoiseLevel_Temporal << 1) + 1) >> 1;
        }

        if (MEDIA_IS_WA(pVeboxState->m_pRenderHal->pWaTable, Wa_1609102037) &&
            VpHalDDIUtils::GetSurfaceColorPack(pVeboxState->m_currentSurface->Format) == VPHAL_COLORPACK_444)
        {
            pVeboxInterfaceXe_Hpm->dw4X4TGNEThCnt = ((pVeboxState->m_currentSurface->dwWidth - 32) *
                                                        (pVeboxState->m_currentSurface->dwHeight - 8)) /
                                                    1600;
        }
        else
        {
            pVeboxInterfaceXe_Hpm->dw4X4TGNEThCnt = ((pVeboxState->m_currentSurface->dwWidth - 8) *
                                                        (pVeboxState->m_currentSurface->dwHeight - 8)) /
                                                    1600;
        }
    }
    else  //first frame or fallback
    {
        dwGNECountLuma = pStatSlice0GNEPtr[3] + pStatSlice1GNEPtr[3];

        // Validate GNE
        if (dwGNELuma == 0xFFFFFFFF || dwGNECountLuma == 0xFFFFFFFF)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
            eStatus = MOS_STATUS_INVALID_PARAMETER;
            goto finish;
        }

        dwGNELuma = dwGNELuma * 100 / (dwGNECountLuma + 1);

        // consistent check
        if (pRenderData->bChromaDenoise)
        {
            GNELumaConsistentCheck(dwGNELuma, pStatSlice0GNEPtr, pStatSlice1GNEPtr);
        }

        if (pDNParams->HVSDenoise.Fallback)
        {
            pVeboxInterfaceXe_Hpm->bHVSfallback  = true;
            pVeboxInterfaceXe_Hpm->dwHistoryInit = 32;
        }
        else
        {
            if (pDNParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE && pDNParams->bEnableHVSDenoise)
            {
                pVeboxInterfaceXe_Hpm->dwHistoryInit = 32;
            }
            pVeboxInterfaceXe_Hpm->bTGNEEnable    = false;
            pVeboxInterfaceXe_Hpm->dwLumaStadTh   = 0;
            pVeboxInterfaceXe_Hpm->dw4X4TGNEThCnt = 0;
        }

        pDNParams->HVSDenoise.dwGlobalNoiseLevel = dwGNELuma;
        pDNParams->HVSDenoise.Sgne_Level         = dwGNELuma;
        pDNParams->HVSDenoise.Sgne_Count         = dwGNECountLuma;
        pDNParams->HVSDenoise.PrevNslvTemporal   = -1;
    }

    // -------------------------- Update Chroma -------------------------------
    // Only use Luma params to substitute Bayer pattern (RGB) DN in Capture Pipe.
    // Chroma params will not be used since there is no chroma.
    if (pRenderData->bChromaDenoise)
    {
        pChromaParams = &ChromaParams;
        MOS_ZeroMemory(pChromaParams, sizeof(VPHAL_DNUV_PARAMS));

        // Setup default Denoise Params for Chroma
        pChromaParams->dwHistoryDeltaUV   = NOISE_HISTORY_DELTA_DEFAULT;
        pChromaParams->dwHistoryMaxUV     = NOISE_HISTORY_MAX_DEFAULT;
        pVeboxDNDIParams->bChromaDNEnable = pRenderData->bChromaDenoise;

        // Combine the GNE in slice0 and slice1 to generate the global GNE and Count
        dwGNEChromaU      = pStatSlice0GNEPtr[1] + pStatSlice1GNEPtr[1];
        dwGNEChromaV      = pStatSlice0GNEPtr[2] + pStatSlice1GNEPtr[2];
        dwGNECountChromaU = pStatSlice0GNEPtr[4] + pStatSlice1GNEPtr[4];
        dwGNECountChromaV = pStatSlice0GNEPtr[5] + pStatSlice1GNEPtr[5];
        dwGNEChromaU      = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
        dwGNEChromaV      = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);

        if (m_bTgneEnable && bTGNE_FirstFrame && !bFirstFrame)
        {
            pVeboxInterfaceXe_Hpm->dwChromaStadTh = 1600;

            if (pDNParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE)
            {
                pVeboxInterfaceXe_Hpm->dwChromaStadTh = 250;

                // Validate GNE
                if (dwGNEChromaU == 0xFFFFFFFF || dwGNECountChromaU == 0xFFFFFFFF ||
                    dwGNEChromaV == 0xFFFFFFFF || dwGNECountChromaV == 0xFFFFFFFF)
                {
                    VPHAL_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
                    eStatus = MOS_STATUS_UNKNOWN;
                    goto finish;
                }
                dwGlobalNoiseLevelU_Temporal = (dwGNEChromaU + 50) / 100;
                dwGlobalNoiseLevelV_Temporal = (dwGNEChromaV + 50) / 100;
            }
            pDNParams->HVSDenoise.PrevNslvTemporalU   = -1;
            pDNParams->HVSDenoise.PrevNslvTemporalV   = -1;
            pDNParams->HVSDenoise.Sgne_CountU         = dwGNECountChromaU;
            pDNParams->HVSDenoise.Sgne_CountV         = dwGNECountChromaV;
            pDNParams->HVSDenoise.Sgne_LevelU         = dwGNEChromaU;
            pDNParams->HVSDenoise.Sgne_LevelV         = dwGNEChromaV;
            pDNParams->HVSDenoise.dwGlobalNoiseLevelU = dwGlobalNoiseLevelU_Temporal;
            pDNParams->HVSDenoise.dwGlobalNoiseLevelV = dwGlobalNoiseLevelV_Temporal;
        }
        else if (m_bTgneEnable && bTGNE_Valid && !bFirstFrame)
        {
            dwGNECountChromaU = (pStatSlice0GNEPtr[4] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[4] & 0x7FFFFFFF);
            dwGNECountChromaV = (pStatSlice0GNEPtr[5] & 0x7FFFFFFF) + (pStatSlice1GNEPtr[5] & 0x7FFFFFFF);

            // Validate TGNE
            if (dwGNECountChromaU == 0 || dwGNECountChromaV == 0)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Incorrect GNE count.");
                eStatus = MOS_STATUS_UNKNOWN;
                goto finish;
            }

            curNoiseLevelU_Temporal = dwGNEChromaU / dwGNECountChromaU;
            curNoiseLevelV_Temporal = dwGNEChromaV / dwGNECountChromaV;

            if (pVeboxInterfaceXe_Hpm->bHVSfallback)
            {
                pDNParams->HVSDenoise.dwGlobalNoiseLevelU = curNoiseLevelU_Temporal;
                pDNParams->HVSDenoise.dwGlobalNoiseLevelV = curNoiseLevelU_Temporal;
                pVeboxInterfaceXe_Hpm->bHVSfallback       = false;
            }
            else
            {
                dwGlobalNoiseLevelU_Temporal              = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevelU_Temporal + curNoiseLevelU_Temporal, 1);
                dwGlobalNoiseLevelV_Temporal              = MOS_ROUNDUP_SHIFT(dwGlobalNoiseLevelV_Temporal + curNoiseLevelV_Temporal, 1);
                pDNParams->HVSDenoise.dwGlobalNoiseLevelU = dwGlobalNoiseLevelU_Temporal;
                pDNParams->HVSDenoise.dwGlobalNoiseLevelV = dwGlobalNoiseLevelV_Temporal;
            }

            sgne_offset  = -12;  //VPHAL_VEBOX_STATISTICS_SURFACE_GNE_OFFSET_G12 - VPHAL_VEBOX_STATISTICS_SURFACE_TGNE_OFFSET_G12;
            dwSgneCountU = pStatSlice0GNEPtr[sgne_offset + 3 + 1] + pStatSlice1GNEPtr[sgne_offset + 3 + 1];
            dwSgneCountV = pStatSlice0GNEPtr[sgne_offset + 3 + 2] + pStatSlice1GNEPtr[sgne_offset + 3 + 2];
            dwSgneU      = pStatSlice0GNEPtr[sgne_offset + 1] + pStatSlice1GNEPtr[sgne_offset + 1];
            dwSgneV      = pStatSlice0GNEPtr[sgne_offset + 2] + pStatSlice1GNEPtr[sgne_offset + 2];

            pDNParams->HVSDenoise.PrevNslvTemporalU = curNoiseLevelU_Temporal;
            pDNParams->HVSDenoise.PrevNslvTemporalV = curNoiseLevelV_Temporal;
            pDNParams->HVSDenoise.Sgne_CountU       = dwSgneCountU;
            pDNParams->HVSDenoise.Sgne_CountV       = dwSgneCountV;
            pDNParams->HVSDenoise.Sgne_LevelU       = dwSgneU * 100 / (dwSgneCountU + 1);
            pDNParams->HVSDenoise.Sgne_LevelV       = dwSgneV * 100 / (dwSgneCountV + 1);

            if (pDNParams->HVSDenoise.Mode == HVSDENOISE_AUTO_BDRATE)
            {
                pVeboxInterfaceXe_Hpm->dwChromaStadTh = 250;
            }
            else
            {
                pVeboxInterfaceXe_Hpm->dwChromaStadTh = (dwGlobalNoiseLevelU_Temporal <= 1 || dwGlobalNoiseLevelV_Temporal <= 1) ? 1600 : (pVeboxInterfaceXe_Hpm->dwChromaStadTh + curNoiseLevelU_Temporal + curNoiseLevelV_Temporal + 1) >> 1;
            }
        }
        else
        {
            dwGNECountChromaU = pStatSlice0GNEPtr[4] + pStatSlice1GNEPtr[4];
            dwGNECountChromaV = pStatSlice0GNEPtr[5] + pStatSlice1GNEPtr[5];

            // Validate GNE
            if (dwGNEChromaU == 0xFFFFFFFF || dwGNECountChromaU == 0xFFFFFFFF ||
                dwGNEChromaV == 0xFFFFFFFF || dwGNECountChromaV == 0xFFFFFFFF)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Incorrect GNE / GNE count.");
                eStatus = MOS_STATUS_UNKNOWN;
                goto finish;
            }

            dwGNEChromaU = dwGNEChromaU * 100 / (dwGNECountChromaU + 1);
            dwGNEChromaV = dwGNEChromaV * 100 / (dwGNECountChromaV + 1);

            if (!pDNParams->HVSDenoise.Fallback)
            {
                pVeboxInterfaceXe_Hpm->dwChromaStadTh = 0;
            }

            pDNParams->HVSDenoise.PrevNslvTemporalU   = -1;
            pDNParams->HVSDenoise.PrevNslvTemporalV   = -1;
            pDNParams->HVSDenoise.Sgne_CountU         = dwGNECountChromaU;
            pDNParams->HVSDenoise.Sgne_CountV         = dwGNECountChromaV;
            pDNParams->HVSDenoise.Sgne_LevelU         = dwGNEChromaU;
            pDNParams->HVSDenoise.Sgne_LevelV         = dwGNEChromaV;
            pDNParams->HVSDenoise.dwGlobalNoiseLevelU = dwGNEChromaU;
            pDNParams->HVSDenoise.dwGlobalNoiseLevelV = dwGNEChromaV;
        }
    }

    VPHAL_VEBOX_STATE_XE_XPM::VeboxSetHVSDNParams(pDNParams);

    if (m_bTgneEnable && bTGNE_FirstFrame && !bFirstFrame)
    {
        bTGNE_FirstFrame = false;
    }

finish:
    return eStatus;
}
