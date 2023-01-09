/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vebox_xe_xpm.cpp
//! \brief    Constructs vebox commands on Xe_XPM platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_vebox_xe_xpm.h"
#include "mhw_vebox_hwcmd_xe_xpm.h"
#include "mhw_utilities_xe_xpm.h"
#include "mos_solo_generic.h"

MhwVeboxInterfaceXe_Xpm::MhwVeboxInterfaceXe_Xpm(PMOS_INTERFACE pOsInterface)
    : MhwVeboxInterfaceG12(pOsInterface)
{
    MHW_FUNCTION_ENTER;
    MEDIA_SYSTEM_INFO *pGtSystemInfo;

    m_veboxScalabilitySupported = false;

    MHW_CHK_NULL_NO_STATUS_RETURN(pOsInterface);
    pGtSystemInfo = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
    MHW_CHK_NULL_NO_STATUS_RETURN(pGtSystemInfo);
    MHW_NORMALMESSAGE("Vebox Number Enabled %d", pGtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled);

    if (pGtSystemInfo->VEBoxInfo.IsValid &&
        pGtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled > 1)
    {
        m_veboxScalabilitySupported = true;
    }
}

MhwVeboxInterfaceXe_Xpm::~MhwVeboxInterfaceXe_Xpm()
{
    MHW_FUNCTION_ENTER;
}

MOS_STATUS MhwVeboxInterfaceXe_Xpm::AddVeboxDiIecp(
    PMOS_COMMAND_BUFFER           pCmdBuffer,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams)
{
    MOS_STATUS          eStatus;
    PMOS_INTERFACE      pOsInterface;
    MHW_RESOURCE_PARAMS ResourceParams;

    mhw_vebox_xe_xpm::VEB_DI_IECP_CMD cmd;
    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(pVeboxDiIecpCmdParams);
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwCurrInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB
    MHW_ASSERT(MOS_IS_ALIGNED(pVeboxDiIecpCmdParams->dwPrevInputSurfOffset, MHW_PAGE_SIZE));  // offset should be aligned with 4KB

    // Initialize
    eStatus      = MOS_STATUS_SUCCESS;
    pOsInterface = m_osInterface;

    if (pVeboxDiIecpCmdParams->pOsResCurrInput)
    {
        if (pVeboxDiIecpCmdParams->CurInputSurfMMCState != MOS_MEMCOMP_DISABLED)
        {
            mhw_vebox_xe_xpm::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD *pSurfCtrlBits;
            pSurfCtrlBits = (mhw_vebox_xe_xpm::VEB_DI_IECP_COMMAND_SURFACE_CONTROL_BITS_CMD*)&pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value;
            pSurfCtrlBits->DW0.MemoryCompressionEnable = 1;
            pSurfCtrlBits->DW0.CompressionType = pSurfCtrlBits->COMPRESSION_TYPE_MEDIACOMPRESSIONENABLED;
            if (pVeboxDiIecpCmdParams->CurInputSurfMMCState == MOS_MEMCOMP_RC)
            {
                pSurfCtrlBits->DW0.CompressionType = pSurfCtrlBits->COMPRESSION_TYPE_RENDERCOMPRESSIONENABLED;
            }
        }

        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResCurrInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->dwCurrInputSurfOffset + pVeboxDiIecpCmdParams->CurrInputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW2.Value);
        ResourceParams.dwLocationInCmd = 2;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResPrevInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResPrevInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->PrevInputSurfCtrl.Value + pVeboxDiIecpCmdParams->dwPrevInputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW4.Value);
        ResourceParams.dwLocationInCmd = 4;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStmmInput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStmmInput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StmmInputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW6.Value);
        ResourceParams.dwLocationInCmd = 6;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStmmOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStmmOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StmmOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW8.Value);
        ResourceParams.dwLocationInCmd = 8;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResDenoisedCurrOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->DenoisedCurrOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW10.Value);
        ResourceParams.dwLocationInCmd = 10;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResCurrOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResCurrOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->CurrOutputSurfCtrl.Value + pVeboxDiIecpCmdParams->dwCurrOutputSurfOffset;
        ResourceParams.pdwCmd          = & (cmd.DW12.Value);
        ResourceParams.dwLocationInCmd = 12;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResPrevOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResPrevOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->PrevOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW14.Value);
        ResourceParams.dwLocationInCmd = 14;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResStatisticsOutput)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResStatisticsOutput;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->StatisticsOutputSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW16.Value);
        ResourceParams.dwLocationInCmd = 16;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResAlphaOrVignette)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResAlphaOrVignette;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->AlphaOrVignetteSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW18.Value);
        ResourceParams.dwLocationInCmd = 18;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResLaceOrAceOrRgbHistogram;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->LaceOrAceOrRgbHistogramSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW20.Value);
        ResourceParams.dwLocationInCmd = 20;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (pVeboxDiIecpCmdParams->pOsResSkinScoreSurface)
    {
        MOS_ZeroMemory(&ResourceParams, sizeof(ResourceParams));
        ResourceParams.presResource    = pVeboxDiIecpCmdParams->pOsResSkinScoreSurface;
        ResourceParams.dwOffset        = pVeboxDiIecpCmdParams->SkinScoreSurfaceSurfCtrl.Value;
        ResourceParams.pdwCmd          = & (cmd.DW22.Value);
        ResourceParams.dwLocationInCmd = 22;
        ResourceParams.bIsWritable     = true;
        ResourceParams.HwCommandType   = MOS_VEBOX_DI_IECP;

        MHW_CHK_STATUS(pfnAddResourceToCmd(
            pOsInterface,
            pCmdBuffer,
            &ResourceParams));
    }

    if (m_veboxScalabilityEnabled == false)
    {
        cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
        cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;
    }
    else
    {
        uint32_t iMediumX;
        MHW_ASSERT(pVeboxDiIecpCmdParams->dwEndingX >= m_numofVebox * 64 - 1);

        iMediumX = MOS_ALIGN_FLOOR(((pVeboxDiIecpCmdParams->dwEndingX + 1) / m_numofVebox), 64);
        iMediumX = MOS_CLAMP_MIN_MAX(iMediumX, 64, (pVeboxDiIecpCmdParams->dwEndingX - 63));

        if (m_numofVebox == 2)
        {
            if (m_indexofVebox == MHW_VEBOX_INDEX_0)
            {
                cmd.DW1.EndingX   = iMediumX - 1;
                cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;
            }
            else if (m_indexofVebox == MHW_VEBOX_INDEX_1)
            {
                cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
                cmd.DW1.StartingX = iMediumX;
            }
            else
            {
                MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
            }
        }
        else if (m_numofVebox == 3)
        {
            if (m_indexofVebox == MHW_VEBOX_INDEX_0)
            {
                cmd.DW1.EndingX   = iMediumX - 1;
                cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;
            }
            else if (m_indexofVebox == MHW_VEBOX_INDEX_1)
            {
                cmd.DW1.EndingX   = 2 * iMediumX - 1;
                cmd.DW1.StartingX = iMediumX;
            }
            else if (m_indexofVebox == MHW_VEBOX_INDEX_2)
            {
                cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
                cmd.DW1.StartingX = 2 * iMediumX;
            }
            else
            {
                MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
            }
        }
        else if (m_numofVebox == 4)
        {
            if (m_indexofVebox == MHW_VEBOX_INDEX_0)
            {
                cmd.DW1.EndingX   = iMediumX - 1;
                cmd.DW1.StartingX = pVeboxDiIecpCmdParams->dwStartingX;
            }
            else if (m_indexofVebox == MHW_VEBOX_INDEX_1)
            {
                cmd.DW1.EndingX   = 2 * iMediumX - 1;
                cmd.DW1.StartingX = iMediumX;
            }
            else if (m_indexofVebox == MHW_VEBOX_INDEX_2)
            {
                cmd.DW1.EndingX   = 3 * iMediumX - 1;
                cmd.DW1.StartingX = 2 * iMediumX;
            }
            else if (m_indexofVebox == MHW_VEBOX_INDEX_3)
            {
                cmd.DW1.EndingX   = pVeboxDiIecpCmdParams->dwEndingX;
                cmd.DW1.StartingX = 3 * iMediumX;
            }
            else
            {
                MHW_ASSERTMESSAGE("Unsupported Vebox Scalability Settings");
            }
        }

        if (m_usingSfc)
        {
            cmd.DW1.SplitWorkloadEnable = true;

            if ((pVeboxDiIecpCmdParams->dwEndingX + 1) != m_numofVebox * iMediumX)
            {
                if (m_indexofVebox < m_numofVebox - 1)
                {
                    cmd.DW1.EndingX   += 64;
                }

                if (m_indexofVebox >= MHW_VEBOX_INDEX_1)
                {
                    cmd.DW1.StartingX += 64;
                }
            }
        }
        else
        {
            cmd.DW1.SplitWorkloadEnable = false;
        }

        cmd.DW24.OutputEndingX      = cmd.DW1.EndingX;
        cmd.DW24.OutputStartingX    = cmd.DW1.StartingX;

        if (m_usingSfc)
        {
            // Use left overfetch for sfc split
            if (cmd.DW1.StartingX >= 64)
            {
                cmd.DW1.StartingX -= 64;
            }
        }

        MT_LOG3(MT_VP_MHW_VE_SCALABILITY, MT_NORMAL, MT_VP_MHW_VE_SCALABILITY_EN, m_veboxScalabilityEnabled,
            MT_VP_MHW_VE_SCALABILITY_USE_SFC, m_usingSfc, MT_VP_MHW_VE_SCALABILITY_IDX, m_indexofVebox);

        MHW_NORMALMESSAGE("VEBOX%d STATE: startx %d endx %d", m_indexofVebox, cmd.DW1.StartingX, cmd.DW1.EndingX);
        MHW_NORMALMESSAGE("VEBOX%d STATE: output startx %d endx %d", m_indexofVebox, cmd.DW24.OutputStartingX, cmd.DW24.OutputEndingX);
    }

    pOsInterface->pfnAddCommand(pCmdBuffer, &cmd, cmd.byteSize);

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceXe_Xpm::VeboxAdjustBoundary(
    PMHW_VEBOX_SURFACE_PARAMS pSurfaceParam,
    uint32_t                  *pdwSurfaceWidth,
    uint32_t                  *pdwSurfaceHeight,
    bool                      bDIEnable,
    bool                      b3DlutEnable)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    MEDIA_FEATURE_TABLE         *pSkuTable = nullptr;

    MHW_CHK_NULL(pSurfaceParam);
    MHW_CHK_NULL(pdwSurfaceWidth);
    MHW_CHK_NULL(pdwSurfaceHeight);
    MHW_CHK_NULL(m_osInterface);

    pSkuTable = m_osInterface->pfnGetSkuTable(m_osInterface);
    MHW_CHK_NULL(pSkuTable);

    MHW_CHK_STATUS(MhwVeboxInterfaceG12::VeboxAdjustBoundary(pSurfaceParam, pdwSurfaceWidth, pdwSurfaceHeight, bDIEnable));

    // match the vebox width with sfc input width to fix corruption issue when sfc scalability enabled in emu
    if (m_veboxScalabilityEnabled && m_usingSfc && m_osInterface->bSimIsActive)
    {
        *pdwSurfaceWidth  = MOS_ALIGN_CEIL(*pdwSurfaceWidth, 16);
        *pdwSurfaceHeight = MOS_ALIGN_CEIL(*pdwSurfaceHeight, 4);
    }

    if (b3DlutEnable && MEDIA_IS_SKU(pSkuTable, FtrHeight8AlignVE3DLUTDualPipe))
    {
        *pdwSurfaceHeight = MOS_ALIGN_CEIL(*pdwSurfaceHeight, 8);
        MHW_NORMALMESSAGE("Align Frame Height as 8x due to 3DlutEnable");
    }

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceXe_Xpm::AddVeboxDndiState(
    PMHW_VEBOX_DNDI_PARAMS pVeboxDndiParams)
{
    PMHW_VEBOX_HEAP pVeboxHeap;
    uint32_t        uiOffset;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD *pVeboxDndiState, mVeboxDndiState;

    MHW_CHK_NULL(pVeboxDndiParams);
    MHW_CHK_NULL(m_veboxHeap);
    pVeboxHeap = m_veboxHeap;

    uiOffset = pVeboxHeap->uiCurState * pVeboxHeap->uiInstanceSize;
    pVeboxDndiState =
        (mhw_vebox_xe_xpm::VEBOX_DNDI_STATE_CMD *)(pVeboxHeap->pLockedDriverResourceMem +
                                                    pVeboxHeap->uiDndiStateOffset +
                                                    uiOffset);
    MHW_CHK_NULL(pVeboxDndiState);
    *pVeboxDndiState = mVeboxDndiState;

    eStatus = MhwVeboxInterfaceG12::AddVeboxDndiState(pVeboxDndiParams);

    if (pVeboxDndiParams->bSCDEnable)
    {
        pVeboxDndiState->DW34.SignBitForMinimumStmm       = 1;
        pVeboxDndiState->DW34.SignBitForMaximumStmm       = 1;
        pVeboxDndiState->DW34.SignBitForSmoothMvThreshold = 1;
    }
    else
    {
        pVeboxDndiState->DW34.SignBitForMinimumStmm       = 0;
        pVeboxDndiState->DW34.SignBitForMaximumStmm       = 0;
        pVeboxDndiState->DW34.SignBitForSmoothMvThreshold = 0;
    }

finish:
    return eStatus;
}

//!
//! \brief    Set which vebox can be used by HW
//! \details  VPHAL set which VEBOX can be use by HW
//! \param    [in] dwVeboxIndex;
//!           set which Vebox can be used by HW
//! \param    [in] dwVeboxCount;
//!           set Vebox Count
//! \param    [in] dwUsingSFC;
//!           set whether using SFC
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
MOS_STATUS MhwVeboxInterfaceXe_Xpm::SetVeboxIndex(
    uint32_t dwVeboxIndex,
    uint32_t dwVeboxCount,
    uint32_t dwUsingSFC)
{
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_ASSERT(dwVeboxIndex < dwVeboxCount);

    m_indexofVebox            = dwVeboxIndex;
    m_numofVebox              = dwVeboxCount;
    m_veboxScalabilityEnabled = (dwVeboxCount > 1) ? m_veboxScalabilitySupported : false;
    m_usingSfc                = dwUsingSFC;

    return eStatus;
}

//!
//! \brief    Create Gpu Context for Vebox
//! \details  Create Gpu Context for Vebox
//! \param    [in] pOsInterface
//!           OS interface
//! \param    [in] VeboxGpuContext
//!           Vebox Gpu Context
//! \param    [in] VeboxGpuNode
//!           Vebox Gpu Node
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS MhwVeboxInterfaceXe_Xpm::CreateGpuContext(
    PMOS_INTERFACE  pOsInterface,
    MOS_GPU_CONTEXT VeboxGpuContext,
    MOS_GPU_NODE    VeboxGpuNode)
{
    MEDIA_FEATURE_TABLE *skuTable;
    MOS_STATUS          eStatus = MOS_STATUS_SUCCESS;
    MOS_GPUCTX_CREATOPTIONS_ENHANCED createOptionenhanced;
    MEDIA_SYSTEM_INFO* pGtSystemInfo = nullptr;

    MHW_CHK_NULL(pOsInterface);

    skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    MHW_CHK_NULL(skuTable);

#if (_DEBUG || _RELEASE_INTERNAL)
    if (MEDIA_IS_SKU(skuTable, FtrContextBasedScheduling) && pOsInterface->bVeboxScalabilityMode)
    {
        pOsInterface->ctxBasedScheduling = true;
    }
#endif
    Mos_SetVirtualEngineSupported(pOsInterface, true);
    pOsInterface->pfnVirtualEngineSupported(pOsInterface, true, true);

    pGtSystemInfo = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
    MHW_CHK_NULL(pGtSystemInfo);

    createOptionenhanced.LRCACount = pGtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled;

    // Create VEBOX/VEBOX2 Context
    MHW_CHK_STATUS(pOsInterface->pfnCreateGpuContext(
        pOsInterface,
        VeboxGpuContext,
        VeboxGpuNode,
        &createOptionenhanced));

finish:
    return eStatus;
}

MOS_STATUS MhwVeboxInterfaceXe_Xpm::AddVeboxSurfaces(
    PMOS_COMMAND_BUFFER                 pCmdBuffer, 
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS pVeboxSurfaceStateCmdParams)
{
    MOS_STATUS eStatus;
    bool       bOutputValid;

    mhw_vebox_xe_xpm::VEBOX_SURFACE_STATE_CMD cmd1, cmd2;

    MHW_CHK_NULL(pCmdBuffer);
    MHW_CHK_NULL(m_osInterface);
    MHW_CHK_NULL(pVeboxSurfaceStateCmdParams);
    MHW_CHK_NULL(m_osInterface->pfnGetMemoryCompressionFormat);

    eStatus = MOS_STATUS_SUCCESS;
    bOutputValid = pVeboxSurfaceStateCmdParams->bOutputValid;

    if (!pVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat)
        m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, pVeboxSurfaceStateCmdParams->SurfInput.pOsResource, &pVeboxSurfaceStateCmdParams->SurfInput.dwCompressionFormat);

    // Setup Surface State for Input surface
    SetVeboxSurfaces(
        &pVeboxSurfaceStateCmdParams->SurfInput,
        &pVeboxSurfaceStateCmdParams->SurfSTMM,
        nullptr,
        &cmd1,
        false,
        pVeboxSurfaceStateCmdParams->bDIEnable,
        pVeboxSurfaceStateCmdParams->b3DlutEnable);
    m_osInterface->pfnAddCommand(pCmdBuffer, &cmd1, cmd1.byteSize);

    // Setup Surface State for Output surface
    if (bOutputValid)
    {
        if (!pVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat)
            m_osInterface->pfnGetMemoryCompressionFormat(m_osInterface, pVeboxSurfaceStateCmdParams->SurfOutput.pOsResource, &pVeboxSurfaceStateCmdParams->SurfOutput.dwCompressionFormat);

        SetVeboxSurfaces(
            &pVeboxSurfaceStateCmdParams->SurfOutput,
            &pVeboxSurfaceStateCmdParams->SurfDNOutput,
            &pVeboxSurfaceStateCmdParams->SurfSkinScoreOutput,
            &cmd2,
            true,
            pVeboxSurfaceStateCmdParams->bDIEnable,
            pVeboxSurfaceStateCmdParams->b3DlutEnable);

        // Reset Output Format When Input/Output Format are the same
        if (pVeboxSurfaceStateCmdParams->SurfInput.Format == pVeboxSurfaceStateCmdParams->SurfOutput.Format)
        {
            cmd2.DW3.SurfaceFormat = cmd1.DW3.SurfaceFormat;
        }

        m_osInterface->pfnAddCommand(pCmdBuffer, &cmd2, cmd2.byteSize);
    }

finish:
    return eStatus;
}

void MhwVeboxInterfaceXe_Xpm::SetVeboxSurfaces(
    PMHW_VEBOX_SURFACE_PARAMS                  pSurfaceParam, 
    PMHW_VEBOX_SURFACE_PARAMS                  pDerivedSurfaceParam, 
    PMHW_VEBOX_SURFACE_PARAMS                  pSkinScoreSurfaceParam, 
    mhw_vebox_xe_xpm::VEBOX_SURFACE_STATE_CMD * pVeboxSurfaceState,
    bool                                       bIsOutputSurface, 
    bool                                       bDIEnable,
    bool                                       b3DlutEnable)
{
    uint32_t dwFormat;
    uint32_t dwSurfaceWidth;
    uint32_t dwSurfaceHeight;
    uint32_t dwSurfacePitch;
    bool     bHalfPitchForChroma;
    bool     bInterleaveChroma;
    uint16_t wUXOffset;
    uint16_t wUYOffset;
    uint16_t wVXOffset;
    uint16_t wVYOffset;
    uint8_t  bBayerOffset;
    uint8_t  bBayerStride;
    uint8_t  bBayerInputAlignment;

    mhw_vebox_xe_xpm::VEBOX_SURFACE_STATE_CMD VeboxSurfaceState;

    MHW_CHK_NULL_NO_STATUS_RETURN(pSurfaceParam);
    MHW_CHK_NULL_NO_STATUS_RETURN(pVeboxSurfaceState);

    // Initialize
    dwSurfaceWidth = 0;
    dwSurfaceHeight = 0;
    dwSurfacePitch = 0;
    bHalfPitchForChroma = false;
    bInterleaveChroma = false;
    wUXOffset = 0;
    wUYOffset = 0;
    wVXOffset = 0;
    wVYOffset = 0;
    bBayerOffset = 0;
    bBayerStride = 0;
    bBayerInputAlignment = 0;
    *pVeboxSurfaceState = VeboxSurfaceState;

    switch (pSurfaceParam->Format)
    {
    case Format_NV12:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR4208;
        bInterleaveChroma = true;
        wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_YUYV:
    case Format_YUY2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBNORMAL;
        break;

    case Format_UYVY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPY;
        break;

    case Format_AYUV:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED444A8;
        break;

    case Format_Y416:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44416;
        break;

    case Format_Y410:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED44410;
        break;

    case Format_YVYU:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUV;
        break;

    case Format_VYUY:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_YCRCBSWAPUVY;
        break;

    case Format_A8B8G8R8:
    case Format_X8B8G8R8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
        break;

    case Format_A16B16G16R16:
    case Format_A16R16G16B16:
    case Format_A16B16G16R16F:
    case Format_A16R16G16B16F:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R16G16B16A16;
        break;

    case Format_L8:
    case Format_P8:
    case Format_Y8:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y8UNORM;
        break;

    case Format_IRW0:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW1:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW3:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_16_BITINPUTATA16_BITSTRIDE;
        break;

    case Format_IRW4:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW5:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW6:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISRED;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_IRW7:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_BAYERPATTERN;
        bBayerOffset = VeboxSurfaceState.BAYER_PATTERN_OFFSET_PIXELATX0_Y0ISGREEN_PIXELATX1_Y0ISBLUE;
        bBayerStride = VeboxSurfaceState.BAYER_PATTERN_FORMAT_8_BITINPUTATA8_BITSTRIDE;
        break;

    case Format_P010:
    case Format_P016:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42016;
        bInterleaveChroma = true;
        wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_A8R8G8B8:
    case Format_X8R8G8B8:
        if (bIsOutputSurface)
        {
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_B8G8R8A8UNORM;
        }
        else
        {
            dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R8G8B8A8UNORMR8G8B8A8UNORMSRGB;
        }
        break;

    case Format_R10G10B10A2:
    case Format_B10G10R10A2:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_R10G10B10A2UNORMR10G10B10A2UNORMSRGB;
        break;

    case Format_Y216:
    case Format_Y210:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PACKED42216;
        break;

    case Format_P216:
    case Format_P210:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_PLANAR42216;
        wUYOffset = (uint16_t)pSurfaceParam->dwUYoffset;
        break;

    case Format_Y16S:
    case Format_Y16U:
        dwFormat = VeboxSurfaceState.SURFACE_FORMAT_Y16UNORM;
        break;

    default:
        MHW_ASSERTMESSAGE("Unsupported format.");
        goto finish;
        break;
    }

    if (!bIsOutputSurface)
    {
        // camera pipe will use 10/12/14 for LSB, 0 for MSB. For other pipeline,
        // dwBitDepth is inherited from pSrc->dwDepth which may not among (0,10,12,14)
        // For such cases should use MSB as default value.
        switch (pSurfaceParam->dwBitDepth)
        {
        case 10:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_10BITLSBALIGNEDDATA;
            break;

        case 12:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_12BITLSBALIGNEDDATA;
            break;

        case 14:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_14BITLSBALIGNEDDATA;
            break;

        case 0:
        default:
            bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
            break;
        }
    }
    else
    {
        bBayerInputAlignment = VeboxSurfaceState.BAYER_INPUT_ALIGNMENT_MSBALIGNEDDATA;
    }

    // adjust boundary for vebox
    VeboxAdjustBoundary(
        pSurfaceParam,
        &dwSurfaceWidth,
        &dwSurfaceHeight,
        bDIEnable,
        b3DlutEnable);

    dwSurfacePitch = (pSurfaceParam->TileType == MOS_TILE_LINEAR) ? MOS_ALIGN_CEIL(pSurfaceParam->dwPitch, MHW_VEBOX_LINEAR_PITCH) : pSurfaceParam->dwPitch;

    pVeboxSurfaceState->DW1.SurfaceIdentification = bIsOutputSurface;
    pVeboxSurfaceState->DW2.Width = dwSurfaceWidth - 1;
    pVeboxSurfaceState->DW2.Height = dwSurfaceHeight - 1;

    pVeboxSurfaceState->DW3.HalfPitchForChroma = bHalfPitchForChroma;
    pVeboxSurfaceState->DW3.InterleaveChroma = bInterleaveChroma;
    pVeboxSurfaceState->DW3.SurfaceFormat = dwFormat;
    pVeboxSurfaceState->DW3.BayerInputAlignment = bBayerInputAlignment;
    pVeboxSurfaceState->DW3.BayerPatternOffset = bBayerOffset;
    pVeboxSurfaceState->DW3.BayerPatternFormat = bBayerStride;
    pVeboxSurfaceState->DW3.SurfacePitch = dwSurfacePitch - 1;
    pVeboxSurfaceState->DW3.TileMode = MosGetHWTileType(pSurfaceParam->TileType, pSurfaceParam->TileModeGMM, pSurfaceParam->bGMMTileEnabled);

    pVeboxSurfaceState->DW4.XOffsetForU = wUXOffset;
    pVeboxSurfaceState->DW4.YOffsetForU = wUYOffset;
    pVeboxSurfaceState->DW5.XOffsetForV = wVXOffset;
    pVeboxSurfaceState->DW5.YOffsetForV = wVYOffset;

    // May fix this for stereo surfaces
    pVeboxSurfaceState->DW6.YOffsetForFrame = pSurfaceParam->dwYoffset;
    pVeboxSurfaceState->DW6.XOffsetForFrame = 0;

    pVeboxSurfaceState->DW7.DerivedSurfacePitch = pDerivedSurfaceParam->dwPitch - 1;
    pVeboxSurfaceState->DW8.SurfacePitchForSkinScoreOutputSurfaces = (bIsOutputSurface && pSkinScoreSurfaceParam->bActive) ? (pSkinScoreSurfaceParam->dwPitch - 1) : 0;

    pVeboxSurfaceState->DW7.CompressionFormat = pSurfaceParam->dwCompressionFormat;

finish:
    return;
}

bool MhwVeboxInterfaceXe_Xpm::IsScalabilitySupported()
{
    return m_veboxScalabilitySupported;
}

MOS_STATUS MhwVeboxInterfaceXe_Xpm::FindVeboxGpuNodeToUse(
    PMHW_VEBOX_GPUNODE_LIMIT pGpuNodeLimit)
{
    MOS_GPU_NODE   VeboxGpuNode = MOS_GPU_NODE_VE;
    MOS_STATUS     eStatus      = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(pGpuNodeLimit);

    // KMD Virtual Engine, use virtual GPU NODE-- MOS_GPU_NODE_VE
    pGpuNodeLimit->dwGpuNodeToUse = VeboxGpuNode;

#if !EMUL
#if (_DEBUG || _RELEASE_INTERNAL)
    if (Mos_Solo_IsInUse(m_osInterface))
    {
        MHW_CHK_STATUS(ValidateVeboxScalabilityConfig());
    }
#endif

    Mos_Solo_CheckNodeLimitation(m_osInterface, &pGpuNodeLimit->dwGpuNodeToUse);
#endif

finish:
    return eStatus;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS MhwVeboxInterfaceXe_Xpm::ValidateVeboxScalabilityConfig()
{
    MEDIA_SYSTEM_INFO *pGtSystemInfo;
    MOS_FORCE_VEBOX eForceVebox;
    bool            bScalableVEMode;
    bool            bUseVE1, bUseVE2, bUseVE3, bUseVE4;
    MOS_STATUS      eStatus = MOS_STATUS_SUCCESS;

    MHW_CHK_NULL(m_osInterface);

    eForceVebox = m_osInterface->eForceVebox;
    bScalableVEMode = ((m_osInterface->bVeboxScalabilityMode) ? true : false);
    pGtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);
    MHW_CHK_NULL(pGtSystemInfo);

    if (eForceVebox != MOS_FORCE_VEBOX_NONE &&
        eForceVebox != MOS_FORCE_VEBOX_1    &&
        eForceVebox != MOS_FORCE_VEBOX_2    &&
        eForceVebox != MOS_FORCE_VEBOX_1_2  &&
        eForceVebox != MOS_FORCE_VEBOX_1_2_3&&
        eForceVebox != MOS_FORCE_VEBOX_1_2_3_4)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("eForceVebox value is invalid.");
        goto finish;
    }

    if (!bScalableVEMode &&
        (eForceVebox == MOS_FORCE_VEBOX_1_2   ||
         eForceVebox == MOS_FORCE_VEBOX_1_2_3 ||
         eForceVebox == MOS_FORCE_VEBOX_1_2_3_4))
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("eForceVebox value is not consistent with scalability mode.");
        goto finish;
    }

    if (bScalableVEMode && !m_veboxScalabilitySupported)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("scalability mode is not allowed on current platform!");
        goto finish;
    }

    bUseVE1 = bUseVE2 = bUseVE3 = bUseVE4 = false;
    if (eForceVebox == MOS_FORCE_VEBOX_NONE)
    {
        bUseVE1 = true;
    }
    else
    {
        MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_1, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE1);
        MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_2, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE2);
        MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_3, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE3);
        MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_4, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE4);
    }

    if (!pGtSystemInfo->VEBoxInfo.IsValid ||
        (uint32_t)(bUseVE1 + bUseVE2 + bUseVE3 + bUseVE4) > pGtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled)
    {
        eStatus = MOS_STATUS_INVALID_PARAMETER;
        MHW_ASSERTMESSAGE("the forced VEBOX is not enabled in current platform.");
        goto finish;
    }

finish:
    return eStatus;
}
#endif
