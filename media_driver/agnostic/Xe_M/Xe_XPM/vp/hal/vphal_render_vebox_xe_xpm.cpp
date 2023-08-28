/*
* Copyright (c) 2019-2022, Intel Corporation
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
//! \file     vphal_render_vebox_xe_xpm.cpp
//! \brief    Interface and structure specific for Xe_XPM Vebox
//! \details  Interface and structure specific for Xe_XPM Vebox
//!
#include "vphal.h"
#include "vphal_render_vebox_base.h"
#include "vphal_render_vebox_xe_xpm.h"
#include "vphal_render_sfc_xe_xpm.h"
#include "vphal_render_vebox_util_base.h"
#include "renderhal_g12_base.h"
#include "hal_oca_interface.h"
#include "mos_interface.h"
#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
#include "igvpkrn_isa_xe_xpm.h"
#include "igvpkrn_isa_xe_hpg.h"
#endif

VPHAL_VEBOX_STATE_XE_XPM::VPHAL_VEBOX_STATE_XE_XPM(
    PMOS_INTERFACE                 pOsInterface,
    PMHW_VEBOX_INTERFACE           pVeboxInterface,
    PMHW_SFC_INTERFACE             pSfcInterface,
    PRENDERHAL_INTERFACE           pRenderHal,
    PVPHAL_VEBOX_EXEC_STATE        pVeboxExecState,
    PVPHAL_RNDR_PERF_DATA          pPerfData,
    const VPHAL_DNDI_CACHE_CNTL    &dndiCacheCntl,
    MOS_STATUS                     *peStatus) :
    VPHAL_VEBOX_STATE(pOsInterface, pVeboxInterface, pSfcInterface, pRenderHal, pVeboxExecState, pPerfData, dndiCacheCntl, peStatus),
    VPHAL_VEBOX_STATE_G12_BASE(pOsInterface, pVeboxInterface, pSfcInterface, pRenderHal, pVeboxExecState, pPerfData, dndiCacheCntl, peStatus)
{
    uint32_t i;
    uint32_t            veboxMaxPipeNum = 0;
    MEDIA_SYSTEM_INFO   *gtSystemInfo    = nullptr;

#if defined(ENABLE_KERNELS) && !defined(_FULL_OPEN_SOURCE)
    m_hvsKernelBinary     = (uint8_t *)IGVPHVS_DENOISE_XE_HPG;
    m_hvsKernelBinarySize = IGVPHVS_DENOISE_XE_HPG_SIZE;
    m_hdr3DLutKernelBinary     = (uint32_t *)IGVP3DLUT_GENERATION_XE_XPM;
    m_hdr3DLutKernelBinarySize = IGVP3DLUT_GENERATION_XE_XPM_SIZE;
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
        VESemaMemSAdd[i] = {};
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

VPHAL_VEBOX_STATE_XE_XPM::~VPHAL_VEBOX_STATE_XE_XPM()
{
    for (auto &icmdBuffer : m_veCmdBuffers)
    {
        if (icmdBuffer)
        {
            MOS_FreeMemory(icmdBuffer);
        }
        icmdBuffer = nullptr;
    }

    if (m_hvsDenoiser)
    {
        MOS_Delete(m_hvsDenoiser);
    }

    m_veCmdBuffers.clear();
    return;
}

VphalSfcState* VPHAL_VEBOX_STATE_XE_XPM::CreateSfcState()
{
#if __VPHAL_SFC_SUPPORTED
    VphalSfcState *sfcState = MOS_New(VphalSfcStateXe_Xpm, m_pOsInterface, m_pRenderHal, m_pSfcInterface);
#else
    VphalSfcState *sfcState = nullptr;
#endif

    return sfcState;
}

VPHAL_OUTPUT_PIPE_MODE VPHAL_VEBOX_STATE_XE_XPM::GetOutputPipe(
    PCVPHAL_RENDER_PARAMS       pcRenderParams,
    PVPHAL_SURFACE              pSrcSurface,
    RenderpassData*             pRenderData)
{
    VPHAL_RENDER_FUNCTION_ENTER;

    VPHAL_OUTPUT_PIPE_MODE      OutputPipe               = VPHAL_OUTPUT_PIPE_MODE_COMP;
    bool                        bHDRToneMappingNeed      = false;
    bool                        bScalingNeeded           = false;

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pcRenderParams);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pRenderData);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(m_sfcPipeState);

    if (pSrcSurface->SurfType == SURF_IN_PRIMARY && pcRenderParams->uSrcCount > 1)
    {
        if (pSrcSurface->Rotation == VPHAL_ROTATION_IDENTITY ||
            pSrcSurface->Rotation == VPHAL_ROTATION_180 ||
            pSrcSurface->Rotation == VPHAL_MIRROR_HORIZONTAL ||
            pSrcSurface->Rotation == VPHAL_MIRROR_VERTICAL)
        {
            bScalingNeeded = ((pSrcSurface->rcDst.right - pSrcSurface->rcDst.left) != (pSrcSurface->rcSrc.right - pSrcSurface->rcSrc.left)) ||
                ((pSrcSurface->rcDst.bottom - pSrcSurface->rcDst.top) != (pSrcSurface->rcSrc.bottom - pSrcSurface->rcSrc.top));
        }
        else
        {
            bScalingNeeded = ((pSrcSurface->rcDst.right - pSrcSurface->rcDst.left) != (pSrcSurface->rcSrc.bottom - pSrcSurface->rcSrc.top)) ||
                ((pSrcSurface->rcDst.bottom - pSrcSurface->rcDst.top) != (pSrcSurface->rcSrc.right - pSrcSurface->rcSrc.left));
        }

        // Using SFC for scaling with the highest priority  due to AVS removal from Render on XeHP, just other SFC has HW limation:
        // input/output surface <128 and unsupport formates will use bilinear or nereast.
        if (bScalingNeeded)
        {
            // enable SFC output temp surf for render enabled cases, make temp surf format the same as input.
            MOS_FORMAT          targetFormat   = pcRenderParams->pTarget[0]->Format;

            pcRenderParams->pTarget[0]->Format = pSrcSurface->Format;
            OutputPipe = m_sfcPipeState->GetOutputPipe(pSrcSurface, pcRenderParams->pTarget[0], pcRenderParams);
            pcRenderParams->pTarget[0]->Format = targetFormat;
            if (OutputPipe == VPHAL_OUTPUT_PIPE_MODE_SFC)
            {
                pRenderData->bSFCScalingOnly = true;
                pRenderData->bCompNeeded = false;

                // Use temp to save input surface's cspace and format; keep the SFC Scaling don't do CSC for multi-layers
                // to save the BW on SFC.
                VPHAL_RENDER_NORMALMESSAGE("Use SFCScalingOnly: input Surface cspace %d, formate %d; output cspace %d, output Format %d",
                  pSrcSurface->ColorSpace, pSrcSurface->Format, pcRenderParams->pTarget[0]->ColorSpace, pcRenderParams->pTarget[0]->Format);
                return OutputPipe;
            }
        }
    }

    // For interlaced scaling, output pipe should be SFC
    if (pSrcSurface->InterlacedScalingType != ISCALING_NONE)
    {
        OutputPipe = m_sfcPipeState->GetOutputPipe(pSrcSurface, pcRenderParams->pTarget[0], pcRenderParams);
        if (OutputPipe == VPHAL_OUTPUT_PIPE_MODE_SFC)
        {
            pRenderData->bCompNeeded = false;
            return OutputPipe;
        }
        else if (OutputPipe == VPHAL_OUTPUT_PIPE_MODE_COMP)
        {
            pRenderData->bCompNeeded = true;
            return OutputPipe;
        }
        else
        {
            return VPHAL_OUTPUT_PIPE_MODE_INVALID;
        }
    }

    return VPHAL_VEBOX_STATE_G12_BASE::GetOutputPipe(pcRenderParams, pSrcSurface, pRenderData);

finish:
    return VPHAL_OUTPUT_PIPE_MODE_INVALID;
}

bool VPHAL_VEBOX_STATE_XE_XPM::IsNeeded(
    PCVPHAL_RENDER_PARAMS pcRenderParams,
    RenderpassData       *pRenderPassData)
{
    PVPHAL_VEBOX_STATE_XE_XPM      pVeboxState = this;
    MhwVeboxInterfaceXe_Xpm *       pVeboxInterfaceXe_Xpm;
    PMHW_VEBOX_INTERFACE              pVeboxInterface;
    PMOS_INTERFACE                    pOsInterface;
    PVPHAL_SURFACE                    pRenderTarget;
    PVPHAL_SURFACE                    pSrcSurface;
    bool                              hr = false;

    // Check whether VEBOX is available
    // VTd doesn't support VEBOX
    if (!MEDIA_IS_SKU(m_pSkuTable, FtrVERing))
    {
        pRenderPassData->bCompNeeded = true;
        goto finish;
    }

    if (pRenderPassData == nullptr || pRenderPassData->pSrcSurface == nullptr)
    {
        VPHAL_RENDER_ASSERTMESSAGE("Invalid input parameter");
        return false;
    }

    // Xe_XPM doesn't support AVS Scaling on Render, HQ scaling should go to SFC.
    // And Ief should only do by SFC.
    // for fast mode, if no vebox feature involved, should go to render path, otherwise goto render path.
    if (!bScalingHQPefMode && (pRenderPassData->pSrcSurface->ScalingPreference == VPHAL_SCALING_PREFER_COMP))
    {
        pRenderPassData->pSrcSurface->ScalingPreference = VPHAL_SCALING_PREFER_SFC;
    }

    VPHAL_RENDER_CHK_NULL_NO_STATUS(pVeboxState);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pVeboxState->m_pVeboxInterface);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pVeboxState->m_pOsInterface);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pcRenderParams);
    VPHAL_RENDER_CHK_NULL_NO_STATUS(pcRenderParams->pTarget[0]);

    if (pcRenderParams->bForceToRender)
    {
        pRenderPassData->bCompNeeded = true;
        goto finish;
    }

    pVeboxInterface         = pVeboxState->m_pVeboxInterface;
    pVeboxInterfaceXe_Xpm = (MhwVeboxInterfaceXe_Xpm *)pVeboxInterface;
    pOsInterface            = pVeboxState->m_pOsInterface;
    pRenderTarget           = pcRenderParams->pTarget[0];
    pSrcSurface             = pRenderPassData->pSrcSurface;

    if (pOsInterface->bVeboxScalabilityMode)
    {
        // use the vebox scalability mode read from registy key if set override
    }
    else
    {
        if (((MOS_MIN(pSrcSurface->dwWidth, (uint32_t)pSrcSurface->rcSrc.right) > MHW_VEBOX_4K_PIC_WIDTH_G12) &&
             (MOS_MIN(pSrcSurface->dwHeight, (uint32_t)pSrcSurface->rcSrc.bottom) > MHW_VEBOX_4K_PIC_HEIGHT_G12)) ||
            ((MOS_MIN(pRenderTarget->dwWidth, (uint32_t)pRenderTarget->rcSrc.right) > MHW_VEBOX_4K_PIC_WIDTH_G12) &&
             (MOS_MIN(pRenderTarget->dwHeight, (uint32_t)pRenderTarget->rcSrc.bottom) > MHW_VEBOX_4K_PIC_HEIGHT_G12)))
        {
            pOsInterface->bVeboxScalabilityMode = pVeboxInterfaceXe_Xpm->m_veboxScalabilitywith4K;
        }
    }

    hr = VPHAL_VEBOX_STATE_G12_BASE::IsNeeded(pcRenderParams, pRenderPassData);

finish:
    return hr;
}

//!
//! \brief    Setup Vebox_DI_IECP Command params for Xe_XPM
//! \details  Setup Vebox_DI_IECP Command params for Xe_XPM
//! \param    [in] bDiScdEnable
//!           Is DI/Variances report enabled
//! \param    [in,out] pVeboxDiIecpCmdParams
//!           Pointer to VEBOX_DI_IECP command parameters
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::SetupDiIecpState(
    bool                          bDiScdEnable,
    PMHW_VEBOX_DI_IECP_CMD_PARAMS pVeboxDiIecpCmdParams)
{
    uint32_t                      dwWidth;
    uint32_t                      dwHeight;
    bool                          bDIEnable;
    MOS_STATUS                    eStatus;
    MHW_VEBOX_SURFACE_PARAMS      MhwVeboxSurfaceParam;
    PMHW_VEBOX_INTERFACE          pVeboxInterface;
    PVPHAL_VEBOX_STATE_XE_XPM   pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA      pRenderData = GetLastExecRenderData();

    VPHAL_RENDER_CHK_NULL(pVeboxDiIecpCmdParams);
    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pRenderData);

    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    MOS_ZeroMemory(pVeboxDiIecpCmdParams, sizeof(*pVeboxDiIecpCmdParams));

    VPHAL_RENDER_CHK_NULL(pVeboxInterface);

    VPHAL_RENDER_CHK_STATUS(VPHAL_VEBOX_STATE_G12_BASE::SetupDiIecpState(bDiScdEnable, pVeboxDiIecpCmdParams));

    // Align dwEndingX with surface state
    bDIEnable = pRenderData->bDeinterlace || IsQueryVarianceEnabled();

    VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceParams(pVeboxState->m_currentSurface, &MhwVeboxSurfaceParam));
    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->VeboxAdjustBoundary(
        &MhwVeboxSurfaceParam,
        &dwWidth,
        &dwHeight,
        bDIEnable));

    pVeboxDiIecpCmdParams->dwStartingX = 0;
    pVeboxDiIecpCmdParams->dwEndingX   = dwWidth - 1;

finish:
    return eStatus;
}

MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::InitCmdBufferWithVeParams(
    PRENDERHAL_INTERFACE             pRenderHal,
    MOS_COMMAND_BUFFER &             CmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS pGenericPrologParams)
{
    MOS_STATUS                          eStatus;
    RENDERHAL_GENERIC_PROLOG_PARAMS_G12 genericPrologParamsG12 = {};
    PVPHAL_VEBOX_RENDER_DATA            pRenderData            = GetLastExecRenderData();
    uint8_t                             i;

    genericPrologParamsG12.bEnableMediaFrameTracking      = pGenericPrologParams->bEnableMediaFrameTracking;
    genericPrologParamsG12.bMmcEnabled                    = pGenericPrologParams->bMmcEnabled;
    genericPrologParamsG12.dwMediaFrameTrackingAddrOffset = pGenericPrologParams->dwMediaFrameTrackingAddrOffset;
    genericPrologParamsG12.dwMediaFrameTrackingTag        = pGenericPrologParams->dwMediaFrameTrackingTag;
    genericPrologParamsG12.presMediaFrameTrackingSurface  = pGenericPrologParams->presMediaFrameTrackingSurface;

    genericPrologParamsG12.VEngineHintParams.BatchBufferCount = dwNumofVebox;

    if (m_veCmdBuffers.size() < dwNumofVebox)
    {
        VPHAL_RENDER_ASSERTMESSAGE("m_veCmdBuffers.size() < dwNumofVebox", m_veCmdBuffers.size(), dwNumofVebox);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (i = 0; i < dwNumofVebox; i++)
    {
        if (!m_veCmdBuffers[i])
        {
            VPHAL_RENDER_ASSERTMESSAGE("m_veCmdBuffers[%d] == nullptr", i);
            return MOS_STATUS_INVALID_PARAMETER;
        }
        genericPrologParamsG12.VEngineHintParams.resScalableBatchBufs[i] = m_veCmdBuffers[i]->OsResource;
        genericPrologParamsG12.VEngineHintParams.EngineInstance[i]       = i;
    }

    genericPrologParamsG12.VEngineHintParams.UsingFrameSplit = true;
    genericPrologParamsG12.VEngineHintParams.UsingSFC        = IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData);

    pRenderHal->pOsInterface->VEEnable = true;

    // Initialize command buffer and insert prolog
    VPHAL_RENDER_CHK_STATUS(pRenderHal->pfnInitCommandBuffer(
        pRenderHal,
        &CmdBuffer,
        (PRENDERHAL_GENERIC_PROLOG_PARAMS)&genericPrologParamsG12));

finish:
    return eStatus;
}

//!
//! \brief    Vebox allocate resources
//! \details  Allocate resources that will be used in Vebox
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::AllocateResources()
{
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE               pOsInterface;
    PVPHAL_VEBOX_STATE_XE_XPM  pVeboxState = this;
    PMHW_VEBOX_INTERFACE         pVeboxInterface;
    MhwVeboxInterfaceXe_Xpm *  pVeboxInterfaceXe_Xpm;
    PVPHAL_VEBOX_RENDER_DATA     pRenderData;
    bool                         bAllocated;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pVeboxInterface);

    pOsInterface    = pVeboxState->m_pOsInterface;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    pRenderData     = GetLastExecRenderData();

    VPHAL_RENDER_CHK_STATUS(VPHAL_VEBOX_STATE_G12_BASE::AllocateResources());

    // Allocate bottom field surface for interleaved to field
    if (pVeboxState->m_currentSurface->InterlacedScalingType == ISCALING_INTERLEAVED_TO_FIELD)
    {
        if (!pRenderData->pOutputTempField)
        {
            pRenderData->pOutputTempField = (VPHAL_SURFACE *)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
        }
        VPHAL_RENDER_CHK_NULL(pRenderData->pOutputTempField);

        VPHAL_RENDER_CHK_STATUS(VpHal_ReAllocateSurface(
            pOsInterface,
            pRenderData->pOutputTempField,
            "OutputBottomFieldSurface_xe_xpm",
            pRenderData->pRenderTarget->Format,
            MOS_GFXRES_2D,
            pRenderData->pRenderTarget->TileType,
            pRenderData->pRenderTarget->dwWidth,
            pRenderData->pRenderTarget->dwHeight,
            pRenderData->pRenderTarget->bIsCompressed,
            pRenderData->pRenderTarget->CompressionMode,
            &bAllocated));
    }
    else
    {
        if (pRenderData->pOutputTempField)
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pRenderData->pOutputTempField->OsResource);

            MOS_FreeMemAndSetNull(pRenderData->pOutputTempField);
        }
    }

    pVeboxInterfaceXe_Xpm = (MhwVeboxInterfaceXe_Xpm *)pVeboxInterface;
    if (pVeboxInterfaceXe_Xpm->IsScalabilitySupported() && pOsInterface->bVeboxScalabilityMode)
    {
        // Allocate resource for Batch buffer to store commands, initialize batch buffer
        VPHAL_RENDER_CHK_STATUS(AllocVESecondaryCmdBuffers());

        // Allocate semaphore resources for Virtual Engine
        VPHAL_RENDER_CHK_STATUS(AllocVESemaphoreResources());
    }
    else
    {
        // Free Virtual Engine related resources
        VPHAL_RENDER_CHK_STATUS(FreeVEResources());
    }

finish:
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        pVeboxState->FreeResources();
    }

    return eStatus;
}

//!
//! \brief    Vebox free resources
//! \details  Free resources that are used in Vebox
//! \return   void
//!
void VPHAL_VEBOX_STATE_XE_XPM::FreeResources()
{
    PVPHAL_VEBOX_STATE_XE_XPM  pVeboxState = this;
    PMOS_INTERFACE               pOsInterface;
    PMHW_VEBOX_INTERFACE         pVeboxInterface;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    int32_t                      i;
    MhwVeboxInterfaceXe_Xpm *  pVeboxInterfaceXe_Xpm;
    PVPHAL_VEBOX_RENDER_DATA     pRenderData;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pVeboxInterface);

    pOsInterface    = pVeboxState->m_pOsInterface;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;
    pRenderData     = GetLastExecRenderData();

    VPHAL_VEBOX_STATE_G12_BASE::FreeResources();

    // Free bottom field surface
    if (pRenderData->pOutputTempField)
    {
        pOsInterface->pfnFreeResource(
            pOsInterface,
            &pRenderData->pOutputTempField->OsResource);

        MOS_FreeMemAndSetNull(pRenderData->pOutputTempField);
    }

    // Free Virtual Engine related resources
    VPHAL_RENDER_CHK_STATUS(FreeVEResources());

finish:
    return;
}

//!
//! \brief    Alloc Batch Buffers with VE interface
//! \details  Allocate Batch Buffers with VE interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::AllocVESecondaryCmdBuffers()
{
    PMOS_INTERFACE               pOsInterface;
    uint32_t                     dwSize;
    int32_t                      i;
    MOS_ALLOC_GFXRES_PARAMS      AllocParamsForBufferLinear;
    MOS_STATUS                   eStatus     = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_STATE_XE_XPM  pVeboxState = this;
    MEDIA_SYSTEM_INFO *          pGtSystemInfo;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);

    pOsInterface = pVeboxState->m_pOsInterface;

    // Initiate allocation paramters
    MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(AllocParamsForBufferLinear));
    AllocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    AllocParamsForBufferLinear.Format   = Format_Buffer;

    // Allocate resource for Batch buffer to store commands
    dwSize                              = MHW_VEBOX_MAX_PIPE_SIZE_G12;
    AllocParamsForBufferLinear.dwBytes  = dwSize;
    AllocParamsForBufferLinear.pBufName = "VEBatchBuffer";

    for (auto &icmdBuffer : pVeboxState->m_veCmdBuffers)
    {
        VPHAL_RENDER_CHK_NULL(icmdBuffer);

        if (Mos_ResourceIsNull(&icmdBuffer->OsResource))
        {
            eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParamsForBufferLinear,
                &icmdBuffer->OsResource);
        }

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate VE Batch Buffer.");
            goto finish;
        }
    }

    pVeboxState->dwVECmdBufSize = dwSize;

#if (_DEBUG || _RELEASE_INTERNAL)
    bool            bUseVE1, bUseVE2, bUseVE3, bUseVE4;
    MOS_FORCE_VEBOX eForceVebox;

    bUseVE1 = bUseVE2 = bUseVE3 = bUseVE4 = false;
    eForceVebox                           = pOsInterface->eForceVebox;

    MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_1, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE1);
    MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_2, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE2);
    MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_3, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE3);
    MHW_VEBOX_IS_VEBOX_SPECIFIED_IN_CONFIG(eForceVebox, MOS_FORCE_VEBOX_4, MOS_FORCEVEBOX_VEBOXID_BITSNUM, MOS_FORCEVEBOX_MASK, bUseVE4);

    pVeboxState->dwNumofVebox = bUseVE1 + bUseVE2 + bUseVE3 + bUseVE4;
    if (pVeboxState->dwNumofVebox == 0)
#endif 
    {
        pGtSystemInfo             = pOsInterface->pfnGetGtSystemInfo(pOsInterface);
        VPHAL_RENDER_CHK_NULL(pGtSystemInfo);
        pVeboxState->dwNumofVebox = pGtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled;
    }

finish:
    return eStatus;
}

//!
//! \brief    Alloc Semaphore Resources with VE interface
//! \details  Allocate Semaphore Resources with VE interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::AllocVESemaphoreResources()
{
    PMOS_INTERFACE               pOsInterface;
    int32_t                      i;
    MOS_ALLOC_GFXRES_PARAMS      AllocParamsForBufferLinear;
    MOS_LOCK_PARAMS              LockFlagsWriteOnly;
    uint8_t *                    pData;
    MOS_STATUS                   eStatus     = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_STATE_XE_XPM  pVeboxState = this;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);

    pOsInterface = pVeboxState->m_pOsInterface;
    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(LockFlagsWriteOnly));
    LockFlagsWriteOnly.WriteOnly = 1;

    // Initiate allocation paramters
    MOS_ZeroMemory(&AllocParamsForBufferLinear, sizeof(AllocParamsForBufferLinear));
    AllocParamsForBufferLinear.Type     = MOS_GFXRES_BUFFER;
    AllocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
    AllocParamsForBufferLinear.Format   = Format_Buffer;

    // Allocate resource for semaphore buffer
    AllocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
    AllocParamsForBufferLinear.pBufName = "VESemaphore";

    for (i = 0; i < MHW_VEBOX_MAX_SEMAPHORE_NUM_G12; i++)
    {
        if (Mos_ResourceIsNull(&pVeboxState->VESemaMemS[i]))
        {
            eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParamsForBufferLinear,
                &pVeboxState->VESemaMemS[i]);
        }

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate VE Semaphore S%d.", i);
            goto finish;
        }

        pData = (uint8_t *)pOsInterface->pfnLockResource(
            pOsInterface,
            &pVeboxState->VESemaMemS[i],
            &LockFlagsWriteOnly);
        VPHAL_RENDER_CHK_NULL(pData);

        MOS_ZeroMemory(pData, sizeof(uint32_t));

        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnUnlockResource(
            pOsInterface,
            &pVeboxState->VESemaMemS[i]));
    }

    // Allocate resource for semaphore buffer addition
    AllocParamsForBufferLinear.dwBytes  = sizeof(uint32_t);
    AllocParamsForBufferLinear.pBufName = "VESemaphoreAdd";

    for (i = 0; i < MHW_VEBOX_MAX_SEMAPHORE_NUM_G12; i++)
    {
        if (Mos_ResourceIsNull(&pVeboxState->VESemaMemSAdd[i]))
        {
            eStatus = (MOS_STATUS)pOsInterface->pfnAllocateResource(
                pOsInterface,
                &AllocParamsForBufferLinear,
                &pVeboxState->VESemaMemSAdd[i]);
        }

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VPHAL_RENDER_ASSERTMESSAGE("Failed to allocate VE Semaphore S%d.", i);
            goto finish;
        }

        pData = (uint8_t *)pOsInterface->pfnLockResource(
            pOsInterface,
            &pVeboxState->VESemaMemSAdd[i],
            &LockFlagsWriteOnly);
        VPHAL_RENDER_CHK_NULL(pData);

        MOS_ZeroMemory(pData, sizeof(uint32_t));

        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnUnlockResource(
            pOsInterface,
            &pVeboxState->VESemaMemSAdd[i]));
    }

finish:
    return eStatus;
}

//!
//! \brief    Init Batch Buffers with VE interface
//! \details  Init Batch Buffers with VE interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::InitVESecondaryCmdBuffers()
{
    PMOS_INTERFACE               pOsInterface;
    MOS_LOCK_PARAMS              LockFlagsWriteOnly;
    uint8_t *                    pData;
    int32_t                      i;
    MOS_STATUS                   eStatus     = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_STATE_XE_XPM  pVeboxState = this;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);

    pOsInterface = pVeboxState->m_pOsInterface;

    MOS_ZeroMemory(&LockFlagsWriteOnly, sizeof(LockFlagsWriteOnly));
    LockFlagsWriteOnly.WriteOnly = 1;

    for (auto &icmdBuffer : pVeboxState->m_veCmdBuffers)
    {
        VPHAL_RENDER_CHK_NULL(icmdBuffer);
        pData = (uint8_t *)pOsInterface->pfnLockResource(
            pOsInterface,
            &icmdBuffer->OsResource,
            &LockFlagsWriteOnly);
        VPHAL_RENDER_CHK_NULL(pData);

        icmdBuffer->pCmdBase   = (uint32_t *)pData;
        icmdBuffer->pCmdPtr    = icmdBuffer->pCmdBase;
        icmdBuffer->iOffset    = 0;
        icmdBuffer->iRemaining = pVeboxState->dwVECmdBufSize;
    }

finish:
    return eStatus;
}

//!
//! \brief    Unlock Batch Buffers with VE interface
//! \details  Unlock Batch Buffers with VE interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::UnLockVESecondaryCmdBuffers()
{
    PMOS_INTERFACE               pOsInterface;
    int32_t                      i;
    MOS_STATUS                   eStatus     = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_STATE_XE_XPM  pVeboxState = this;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);

    pOsInterface = pVeboxState->m_pOsInterface;

    for (auto &icmdBuffer : pVeboxState->m_veCmdBuffers)
    {
        VPHAL_RENDER_CHK_NULL(icmdBuffer);

        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnUnlockResource(
            pOsInterface,
            &icmdBuffer->OsResource));
    }

finish:
    return eStatus;
}

//!
//! \brief    Free Resources with VE interface
//! \details  Free Resources with VE interface
//! \return   MOS_STATUS
//!           MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::FreeVEResources()
{
    PMOS_INTERFACE               pOsInterface;
    PVPHAL_VEBOX_STATE_XE_XPM  pVeboxState = this;
    PMHW_VEBOX_INTERFACE         pVeboxInterface;
    MOS_STATUS                   eStatus = MOS_STATUS_SUCCESS;
    int32_t                      i;
    MhwVeboxInterfaceXe_Xpm *  pVeboxInterfaceXe_Xpm;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pVeboxInterface);

    pOsInterface    = pVeboxState->m_pOsInterface;
    pVeboxInterface = pVeboxState->m_pVeboxInterface;

    // Free Virtual Engine related resources
    pVeboxInterfaceXe_Xpm = (MhwVeboxInterfaceXe_Xpm *)pVeboxInterface;
    if (pVeboxInterfaceXe_Xpm->IsScalabilitySupported())
    {
        for (auto &icmdBuffer : pVeboxState->m_veCmdBuffers)
        {
            if (!icmdBuffer)
            {
                VPHAL_RENDER_ASSERTMESSAGE("Invalid icmdBuffer");

                continue;
            }
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &icmdBuffer->OsResource);
        }

        for (i = 0; i < MHW_VEBOX_MAX_SEMAPHORE_NUM_G12; i++)
        {
            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pVeboxState->VESemaMemS[i]);

            pOsInterface->pfnFreeResource(
                pOsInterface,
                &pVeboxState->VESemaMemSAdd[i]);
        }
    }

finish:
    return eStatus;
}

MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::VeboxRenderVeboxCmd(
    MOS_COMMAND_BUFFER &                  CmdBuffer,
    MHW_VEBOX_DI_IECP_CMD_PARAMS &        VeboxDiIecpCmdParams,
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS &VeboxSurfaceStateCmdParams,
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS &  MhwVeboxSurfaceStateCmdParams,
    MHW_VEBOX_STATE_CMD_PARAMS &          VeboxStateCmdParams,
    MHW_MI_FLUSH_DW_PARAMS &              FlushDwParams,
    PRENDERHAL_GENERIC_PROLOG_PARAMS      pGenericPrologParams)
{
    MOS_STATUS                        eStatus     = MOS_STATUS_SUCCESS;
    PVPHAL_VEBOX_RENDER_DATA          pRenderData = GetLastExecRenderData();
    MhwVeboxInterfaceXe_Xpm *         pVeboxInterfaceXe_Xpm;
    PMHW_VEBOX_INTERFACE              pVeboxInterface;
    MHW_MI_ATOMIC_PARAMS              AtomicParams;
    MHW_MI_SEMAPHORE_WAIT_PARAMS      MiSemaphoreWaitParams;
    PVPHAL_VEBOX_STATE_XE_XPM         pVeboxState = this;
    PRENDERHAL_INTERFACE_LEGACY       pRenderHal;
    bool                              bDiVarianceEnable;
    PMOS_INTERFACE                    pOsInterface;
    uint32_t                          IdxofVebox;
    PMOS_COMMAND_BUFFER               pCmdBufferInUse;
    MHW_GENERIC_PROLOG_PARAMS         genericPrologParams;
    PMHW_MI_INTERFACE                 pMhwMiInterface;
    MHW_MI_LOAD_REGISTER_IMM_PARAMS   RegisterImmParams;
    const MHW_VEBOX_HEAP *            pVeboxHeap = nullptr;
    MOS_CONTEXT                       *pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS             pMmioRegisters = nullptr;
    MOS_COMMAND_BUFFER                CmdBufferInUse = {};
    bool                              veboxEnableScalability = true;

    VPHAL_RENDER_CHK_NULL(pVeboxState);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pOsInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pVeboxInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal->pMhwMiInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal->pMhwMiInterface->GetMmioRegisters());
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal->pOsInterface);
    VPHAL_RENDER_CHK_NULL(pVeboxState->m_pRenderHal->pOsInterface->pOsContext);

    pVeboxInterface         = pVeboxState->m_pVeboxInterface;
    pRenderHal              = pVeboxState->m_pRenderHal;
    pOsInterface            = pVeboxState->m_pOsInterface;
    pMhwMiInterface         = pRenderHal->pMhwMiInterface;
    pVeboxInterfaceXe_Xpm = (MhwVeboxInterfaceXe_Xpm *)pVeboxInterface;
    pOsContext              = pOsInterface->pOsContext;
    pMmioRegisters          = pMhwMiInterface->GetMmioRegisters();

    VPHAL_RENDER_CHK_STATUS(pVeboxInterface->GetVeboxHeapInfo(
        &pVeboxHeap));
    VPHAL_RENDER_CHK_NULL(pVeboxHeap);
    VPHAL_RENDER_CHK_NULL(pGenericPrologParams);

    if (pOsInterface->bVeboxScalabilityMode)
    {
        // get configuration value
        pVeboxState->bVeboxScalableMode = true;
    }
    else
    {
        if (pVeboxInterfaceXe_Xpm->m_veboxScalabilitywith4K)
        {
            if (((MOS_MIN(m_currentSurface->dwWidth, (uint32_t)m_currentSurface->rcSrc.right) > MHW_VEBOX_4K_PIC_WIDTH_G12) &&
                (MOS_MIN(m_currentSurface->dwHeight, (uint32_t)m_currentSurface->rcSrc.bottom)> MHW_VEBOX_4K_PIC_HEIGHT_G12)) ||
                ((MOS_MIN(pRenderData->pRenderTarget->dwWidth, (uint32_t)pRenderData->pRenderTarget->rcSrc.right) > MHW_VEBOX_4K_PIC_WIDTH_G12) &&
                (MOS_MIN(pRenderData->pRenderTarget->dwHeight, (uint32_t)pRenderData->pRenderTarget->rcSrc.bottom)> MHW_VEBOX_4K_PIC_HEIGHT_G12)))
            {
                bVeboxScalableMode = ((pVeboxInterfaceXe_Xpm->IsScalabilitySupported() == true) ? true : false);
            }
            else
            {
                bVeboxScalableMode = false;
            }
        }
        else
        {
            bVeboxScalableMode = pOsInterface->bVeboxScalabilityMode;
        }
    }

    if (pVeboxInterfaceXe_Xpm->IsScalabilitySupported() &&
        pVeboxState->bVeboxScalableMode &&
        (m_currentSurface->dwWidth > dwNumofVebox * 64) &&
        (pRenderData->pRenderTarget->dwWidth > dwNumofVebox * 64))
    {
        if (CmdBuffer.Attributes.pAttriVe == nullptr)
        {
            if (pOsInterface->apoMosEnabled)
            {
                VPHAL_RENDER_CHK_NULL(pOsInterface->osStreamState);
                VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnSetupAttributeVeBuffer(pOsInterface->osStreamState, &CmdBuffer));
            }
            CmdBuffer.Attributes.pAttriVe = &pOsInterface->bufAttriVe[pOsInterface->CurrentGpuContextOrdinal];
        }

#ifdef _MMC_SUPPORTED
        VPHAL_RENDER_CHK_STATUS(pVeboxInterfaceXe_Xpm->setVeboxPrologCmd(m_pRenderHal->pMhwMiInterface, &CmdBuffer));
#endif

        // Initialize command buffer and insert prolog with VE params
        VPHAL_RENDER_CHK_STATUS(InitCmdBufferWithVeParams(pRenderHal, CmdBuffer, pGenericPrologParams));

        // Initialize the scalability state
        pVeboxInterfaceXe_Xpm->SetVeboxIndex(0, dwNumofVebox, IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData));

        bDiVarianceEnable = pRenderData->bDeinterlace || IsQueryVarianceEnabled();

        pVeboxState->SetupSurfaceStates(
            bDiVarianceEnable,
            &VeboxSurfaceStateCmdParams);

        pVeboxState->SetupVeboxState(
            bDiVarianceEnable,
            &VeboxStateCmdParams);

        // Ensure LACE LUT table is ready to be read
        if (VeboxStateCmdParams.pLaceLookUpTables)
        {
            pOsInterface->pfnSyncOnResource(
                pOsInterface,
                VeboxStateCmdParams.pLaceLookUpTables,
                MOS_GPU_CONTEXT_VEBOX,
                false);
        }

        VPHAL_RENDER_CHK_STATUS(pVeboxState->SetupDiIecpState(
            bDiVarianceEnable,
            &VeboxDiIecpCmdParams));

        VPHAL_RENDER_CHK_STATUS(pVeboxState->VeboxIsCmdParamsValid(
            VeboxStateCmdParams,
            VeboxDiIecpCmdParams,
            VeboxSurfaceStateCmdParams));

        // Ensure output is ready to be written
        if (VeboxDiIecpCmdParams.pOsResCurrOutput)
        {
            pOsInterface->pfnSyncOnResource(
                pOsInterface,
                VeboxDiIecpCmdParams.pOsResCurrOutput,
                MOS_GPU_CONTEXT_VEBOX,
                true);

            // Synchronize overlay if overlay is used because output could be Render Target
            if (VeboxSurfaceStateCmdParams.pSurfOutput &&
                VeboxSurfaceStateCmdParams.pSurfOutput->bOverlay)
            {
                pOsInterface->pfnSyncOnOverlayResource(
                    pOsInterface,
                    VeboxDiIecpCmdParams.pOsResCurrOutput,
                    MOS_GPU_CONTEXT_VEBOX);
            }
        }

        if (VeboxDiIecpCmdParams.pOsResPrevOutput)
        {
            pOsInterface->pfnSyncOnResource(
                pOsInterface,
                VeboxDiIecpCmdParams.pOsResPrevOutput,
                MOS_GPU_CONTEXT_VEBOX,
                true);
        }

        if (VeboxDiIecpCmdParams.pOsResDenoisedCurrOutput)
        {
            pOsInterface->pfnSyncOnResource(
                pOsInterface,
                VeboxDiIecpCmdParams.pOsResDenoisedCurrOutput,
                MOS_GPU_CONTEXT_VEBOX,
                true);
        }

        if (VeboxDiIecpCmdParams.pOsResStatisticsOutput)
        {
            pOsInterface->pfnSyncOnResource(
                pOsInterface,
                VeboxDiIecpCmdParams.pOsResStatisticsOutput,
                MOS_GPU_CONTEXT_VEBOX,
                true);
        }

        VPHAL_RENDER_CHK_STATUS(VpHal_InitVeboxSurfaceStateCmdParams(
            &VeboxSurfaceStateCmdParams, &MhwVeboxSurfaceStateCmdParams));

        // InitVESecondaryCmdBuffers() should be on parity with UnLockVESecondaryCmdBuffer() since InitVESecondaryCmdBuffers() contains Lock.
        VPHAL_RENDER_CHK_STATUS(InitVESecondaryCmdBuffers());

        if (pVeboxState->m_veCmdBuffers.size() < dwNumofVebox)
        {
            VPHAL_RENDER_ASSERTMESSAGE("pVeboxState->m_veCmdBuffers.size() (%d) < dwNumofVebox(%d)", pVeboxState->m_veCmdBuffers.size(), dwNumofVebox);
            return MOS_STATUS_INVALID_PARAMETER;
        }

        for (IdxofVebox = 0; IdxofVebox < dwNumofVebox; IdxofVebox++)
        {
            if (pOsInterface->bParallelSubmission)
            {
                // initialize the command buffer struct
                MOS_ZeroMemory(&CmdBufferInUse, sizeof(MOS_COMMAND_BUFFER));
                VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnGetCommandBuffer(pOsInterface, &CmdBufferInUse, 1 + IdxofVebox));
                pCmdBufferInUse = &CmdBufferInUse;
            }
            else
            {
                pCmdBufferInUse = pVeboxState->m_veCmdBuffers[IdxofVebox];
            }
            VPHAL_RENDER_CHK_NULL(pCmdBufferInUse);

            pVeboxInterfaceXe_Xpm->SetVeboxIndex(IdxofVebox, dwNumofVebox, IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData));

            HalOcaInterface::On1stLevelBBStart(*pCmdBufferInUse, *pOsContext, pOsInterface->CurrentGpuContextHandle,
                *pRenderHal->pMhwMiInterface, *pMmioRegisters);

            // Add vphal param to log.
            HalOcaInterface::DumpVphalParam(*pCmdBufferInUse, (MOS_CONTEXT_HANDLE)pOsContext, pRenderHal->pVphalOcaDumper);

            // Profiler start cmd
            if (pRenderHal->pPerfProfiler)
            {
                VPHAL_RENDER_CHK_STATUS(pRenderHal->pPerfProfiler->AddPerfCollectStartCmd((void*)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, pCmdBufferInUse));
            }

            VPHAL_RENDER_CHK_STATUS(NullHW::StartPredicate(pOsInterface, pRenderHal->pMhwMiInterface, pCmdBufferInUse));

            // Insert prolog with VE params
            VPHAL_RENDER_CHK_STATUS(pVeboxInterfaceXe_Xpm->setVeboxPrologCmd(m_pRenderHal->pMhwMiInterface, pCmdBufferInUse));
            MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
            genericPrologParams.pOsInterface  = pRenderHal->pOsInterface;
            genericPrologParams.pvMiInterface = pRenderHal->pMhwMiInterface;
            genericPrologParams.bMmcEnabled   = pGenericPrologParams->bMmcEnabled;
            MHW_RENDERHAL_CHK_STATUS(Mhw_SendGenericPrologCmd(pCmdBufferInUse, &genericPrologParams));

            for (uint32_t tmpIdx = 0; tmpIdx < dwNumofVebox; tmpIdx++)
            {
                // MI Atomic S[All] Increase 1
                MOS_ZeroMemory(&AtomicParams, sizeof(AtomicParams));
                AtomicParams.pOsResource       = &pVeboxState->VESemaMemS[tmpIdx];
                AtomicParams.dwDataSize        = sizeof(uint32_t);
                AtomicParams.Operation         = MHW_MI_ATOMIC_INC;
                AtomicParams.bInlineData       = true;
                AtomicParams.dwOperand1Data[0] = 1;
                VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiAtomicCmd(pCmdBufferInUse, &AtomicParams));
            }

            // MI Atomic S[IdxofVebox] Wait dwNumofVebox
            MOS_ZeroMemory(&MiSemaphoreWaitParams, sizeof(MiSemaphoreWaitParams));
            MiSemaphoreWaitParams.presSemaphoreMem = &pVeboxState->VESemaMemS[IdxofVebox];
            MiSemaphoreWaitParams.bPollingWaitMode = true;
            MiSemaphoreWaitParams.dwSemaphoreData  = dwNumofVebox;
            MiSemaphoreWaitParams.CompareOperation = MHW_MI_SAD_EQUAL_SDD;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiSemaphoreWaitCmd(pCmdBufferInUse, &MiSemaphoreWaitParams));

            // Enable Watchdog Timer
            MOS_ZeroMemory(&RegisterImmParams, sizeof(RegisterImmParams));
            if (pRenderData->pRenderTarget->dwWidth * pRenderData->pRenderTarget->dwHeight > MHW_VEBOX_4K_PIC_WIDTH_G12 * MHW_VEBOX_4K_PIC_HEIGHT_G12 * dwNumofVebox)
            {
                RegisterImmParams.dwData     = MHW_VEBOX_TIMESTAMP_CNTS_PER_SEC_G12 * MHW_VEBOX_TIMEOUT_MS / 333;
            }
            else
            {
                RegisterImmParams.dwData     = MHW_VEBOX_TIMESTAMP_CNTS_PER_SEC_G12 * MHW_VEBOX_TIMEOUT_MS / 1000;
            }
            RegisterImmParams.dwRegister = WATCHDOG_COUNT_THRESTHOLD_OFFSET_VECS_G12;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiLoadRegisterImmCmd(
                pCmdBufferInUse,
                &RegisterImmParams));

            RegisterImmParams.dwData     = MHW_VEBOX_WATCHDOG_ENABLE_COUNTER;
            RegisterImmParams.dwRegister = WATCHDOG_COUNT_CTRL_OFFSET_VECS_G12;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiLoadRegisterImmCmd(
                pCmdBufferInUse,
                &RegisterImmParams));

#if (_DEBUG || _RELEASE_INTERNAL)
            // Add noop for simu no output issue
            if (IdxofVebox > 0)
            {
                pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
                {
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                }
            }
#endif

            //---------------------------------
            // Send CMD: Vebox_State
            //---------------------------------
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxState(
                pCmdBufferInUse,
                &VeboxStateCmdParams,
                0));

            //---------------------------------
            // Send CMD: Vebox_Surface_State
            //---------------------------------
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxSurfaces(
                pCmdBufferInUse,
                &MhwVeboxSurfaceStateCmdParams));

            //---------------------------------
            // Send CMD: SFC pipe commands
            //---------------------------------
            if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
            {
                m_sfcPipeState->SetSfcIndex(IdxofVebox, dwNumofVebox);
                VPHAL_RENDER_CHK_STATUS(m_sfcPipeState->SendSfcCmd(
                    pRenderData,
                    pCmdBufferInUse));
            }

            HalOcaInterface::OnDispatch(*pCmdBufferInUse, *pOsInterface, *pRenderHal->pMhwMiInterface, *pMmioRegisters);

            //---------------------------------
            // Send CMD: Vebox_DI_IECP
            //---------------------------------
            VPHAL_RENDER_CHK_STATUS(pVeboxInterface->AddVeboxDiIecp(
                pCmdBufferInUse,
                &VeboxDiIecpCmdParams));

            // MI FlushDw, for vebox output green block issue
            MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiFlushDwCmd(pCmdBufferInUse, &FlushDwParams));

            for (uint32_t tmpIdx = 0; tmpIdx < dwNumofVebox; tmpIdx++)
            {
                // MI Atomic SAdd[All] Increase 1
                MOS_ZeroMemory(&AtomicParams, sizeof(AtomicParams));
                AtomicParams.pOsResource       = &pVeboxState->VESemaMemSAdd[tmpIdx];
                AtomicParams.dwDataSize        = sizeof(uint32_t);
                AtomicParams.Operation         = MHW_MI_ATOMIC_INC;
                AtomicParams.bInlineData       = true;
                AtomicParams.dwOperand1Data[0] = 1;
                VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiAtomicCmd(pCmdBufferInUse, &AtomicParams));
            }

            // MI Atomic SAdd[IdxofVebox] Wait dwNumofVebox
            MOS_ZeroMemory(&MiSemaphoreWaitParams, sizeof(MiSemaphoreWaitParams));
            MiSemaphoreWaitParams.presSemaphoreMem = &pVeboxState->VESemaMemSAdd[IdxofVebox];
            MiSemaphoreWaitParams.bPollingWaitMode = true;
            MiSemaphoreWaitParams.dwSemaphoreData  = dwNumofVebox;
            MiSemaphoreWaitParams.CompareOperation = MHW_MI_SAD_EQUAL_SDD;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiSemaphoreWaitCmd(pCmdBufferInUse, &MiSemaphoreWaitParams));

            //---------------------------------
            // Write GPU Status Tag for Tag based synchronization
            //---------------------------------
            if (!pOsInterface->bEnableKmdMediaFrameTracking)
            {
                VPHAL_RENDER_CHK_STATUS(VeboxSendVecsStatusTag(
                    pMhwMiInterface,
                    pOsInterface,
                    pCmdBufferInUse));
            }

            //---------------------------------
            // Write Sync tag for Vebox Heap Synchronization
            // If KMD frame tracking is on, the synchrinization of Vebox Heap will use Status tag which
            // is updated using KMD frame tracking.
            //---------------------------------
            if (!pOsInterface->bEnableKmdMediaFrameTracking)
            {
                MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
                FlushDwParams.pOsResource      = (PMOS_RESOURCE)&pVeboxHeap->DriverResource;
                FlushDwParams.dwResourceOffset = pVeboxHeap->uiOffsetSync;
                FlushDwParams.dwDataDW1        = pVeboxHeap->dwNextTag;
                VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiFlushDwCmd(
                    pCmdBufferInUse,
                    &FlushDwParams));
            }

            // Disable Watchdog Timer
            RegisterImmParams.dwData     = MHW_VEBOX_WATCHDOG_DISABLE_COUNTER;
            RegisterImmParams.dwRegister = WATCHDOG_COUNT_CTRL_OFFSET_VECS_G12;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiLoadRegisterImmCmd(
                pCmdBufferInUse,
                &RegisterImmParams));

            // MI Atomic S[IdxofVebox] Reset
            MHW_MI_STORE_DATA_PARAMS dataParams = {};
            dataParams.pOsResource      = &pVeboxState->VESemaMemS[IdxofVebox];
            dataParams.dwResourceOffset = 0;
            dataParams.dwValue          = 0;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiStoreDataImmCmd(
                pCmdBufferInUse, &dataParams));

            // MI Atomic SAdd[IdxofVebox] Reset
            dataParams.pOsResource      = &pVeboxState->VESemaMemSAdd[IdxofVebox];
            dataParams.dwResourceOffset = 0;
            dataParams.dwValue          = 0;
            VPHAL_RENDER_CHK_STATUS(pMhwMiInterface->AddMiStoreDataImmCmd(
                pCmdBufferInUse, &dataParams));

            VPHAL_RENDER_CHK_STATUS(NullHW::StopPredicate(pOsInterface, pRenderHal->pMhwMiInterface, &CmdBuffer));

            // Profiler end cmd
            if (pRenderHal->pPerfProfiler)
            {
                VPHAL_RENDER_CHK_STATUS(pRenderHal->pPerfProfiler->AddPerfCollectEndCmd((void*)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, pCmdBufferInUse));
            }

            HalOcaInterface::On1stLevelBBEnd(*pCmdBufferInUse, *pOsInterface);

            VPHAL_RENDER_CHK_STATUS(AddMiBatchBufferEnd(pOsInterface, pMhwMiInterface, pCmdBufferInUse));

            // Submit the secondary cmdbuffer seperately
            if (pOsInterface->phasedSubmission)
            {
                int32_t submissiontype[4] = {SUBMISSION_TYPE_MULTI_PIPE_MASTER,
                    SUBMISSION_TYPE_MULTI_PIPE_SLAVE,
                    SUBMISSION_TYPE_MULTI_PIPE_SLAVE,
                    SUBMISSION_TYPE_MULTI_PIPE_SLAVE};

                pCmdBufferInUse->iSubmissionType = submissiontype[IdxofVebox];
                pCmdBufferInUse->iVeboxNodeIndex = (MOS_VEBOX_NODE_IND)IdxofVebox;
                bPhasedSubmission                = true;

                if (IdxofVebox == dwNumofVebox - 1)
                {
                    pCmdBufferInUse->iSubmissionType |= SUBMISSION_TYPE_MULTI_PIPE_FLAGS_LAST_PIPE;
                }

                if (pOsInterface->bParallelSubmission)
                {
                    // Return unused command buffer space to OS
                    pOsInterface->pfnReturnCommandBuffer(
                        pOsInterface,
                        &CmdBufferInUse,
                        1 + IdxofVebox);

                    if (IdxofVebox == dwNumofVebox - 1)
                    {
                        VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnSubmitCommandBuffer(
                            pOsInterface,
                            pCmdBufferInUse,
                            pVeboxState->bNullHwRenderDnDi));
                    }
                }
                else
                {
                    VPHAL_RENDER_CHK_STATUS(pOsInterface->pfnSubmitCommandBuffer(
                        pOsInterface,
                        pCmdBufferInUse,
                        pVeboxState->bNullHwRenderDnDi));
                }
            }
            else
            {
                bPhasedSubmission = false;
            }
        }

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED  // dump the secondry command buffers for scalable vebox troubleshoot.
        if (pOsInterface->bDumpCommandBuffer && pOsInterface->pfnDumpCommandBuffer)
        {
            if (pVeboxState->m_veCmdBuffers.size() < dwNumofVebox)
            {
                VPHAL_RENDER_ASSERTMESSAGE("pVeboxState->m_veCmdBuffers.size() (%d) < dwNumofVebox(%d)", pVeboxState->m_veCmdBuffers.size(), dwNumofVebox);
                return MOS_STATUS_INVALID_PARAMETER;
            }
            for (IdxofVebox = 0; IdxofVebox < dwNumofVebox; IdxofVebox++)
            {
                pOsInterface->pfnDumpCommandBuffer(pOsInterface, pVeboxState->m_veCmdBuffers[IdxofVebox]);
            }
        }
#endif  // MOS_COMMAND_BUFFER_DUMP_SUPPORTED

        VPHAL_RENDER_CHK_STATUS(UnLockVESecondaryCmdBuffers());

#if (_DEBUG || _RELEASE_INTERNAL)
        ReportUserSetting(
            m_userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE,
            veboxEnableScalability,
            MediaUserSetting::Group::Device);
#endif
    }
    else
    {
        CmdBuffer.iSubmissionType = SUBMISSION_TYPE_SINGLE_PIPE;
        bPhasedSubmission         = false;

        if (CmdBuffer.Attributes.pAttriVe)
        {
            PMOS_CMD_BUF_ATTRI_VE pAttriVe = (PMOS_CMD_BUF_ATTRI_VE)(CmdBuffer.Attributes.pAttriVe);
            pAttriVe->bUseVirtualEngineHint = false;
        }

        pVeboxInterfaceXe_Xpm->SetVeboxIndex(0, 1, IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData));
        if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))
        {
            m_sfcPipeState->SetSfcIndex(0, 1);
        }

        VPHAL_RENDER_CHK_STATUS(VPHAL_VEBOX_STATE_G12_BASE::VeboxRenderVeboxCmd(
            CmdBuffer,
            VeboxDiIecpCmdParams,
            VeboxSurfaceStateCmdParams,
            MhwVeboxSurfaceStateCmdParams,
            VeboxStateCmdParams,
            FlushDwParams,
            pGenericPrologParams));

        veboxEnableScalability = false;
#if (_DEBUG || _RELEASE_INTERNAL)
        ReportUserSetting(
            m_userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE,
            veboxEnableScalability,
            MediaUserSetting::Group::Device);
#endif
    }

finish:
    return eStatus;
}


MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::AddMiBatchBufferEnd(
    PMOS_INTERFACE      pOsInterface,
    PMHW_MI_INTERFACE   pMhwMiInterface,
    PMOS_COMMAND_BUFFER pCmdBufferInUse)
{
    if (pOsInterface->bNoParsingAssistanceInKmd)
    {
        return pMhwMiInterface->AddMiBatchBufferEnd(
            pCmdBufferInUse,
            nullptr);
    }
    return MOS_STATUS_SUCCESS;
}

PVPHAL_SURFACE VPHAL_VEBOX_STATE_XE_XPM::GetSurfOutput(bool bDiVarianceEnable)
{
    PVPHAL_SURFACE              pSurface = nullptr;
    PVPHAL_VEBOX_STATE_G12_BASE pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA    pRenderData = GetLastExecRenderData();

    if (IS_VPHAL_OUTPUT_PIPE_VEBOX(pRenderData))                    // Vebox output pipe
    {
        pSurface = pRenderData->pRenderTarget;
    }
    else if (bDiVarianceEnable)                                     // DNDI, DI, DI + IECP
    {
        pSurface = pVeboxState->FFDISurfaces[pRenderData->iFrame0];
    }
    else if (IsIECPEnabled())                                       // DN + IECP or IECP only
    {
        pSurface = pVeboxState->FFDISurfaces[pRenderData->iCurDNOut];
    }
    else if (pRenderData->bDenoise)                                 // DN only
    {
        if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData) && !bDiVarianceEnable)
        {
            pSurface = pVeboxState->FFDISurfaces[pRenderData->iCurDNOut];
        }
        else
        {
            pSurface = pVeboxState->FFDNSurfaces[pRenderData->iCurDNOut];
        }
    }
    else if (IS_VPHAL_OUTPUT_PIPE_SFC(pRenderData))                 // Write to SFC
    {
        // Vebox o/p should not be written to memory
        pSurface = nullptr;
    }
    else
    {
        VPHAL_RENDER_ASSERTMESSAGE("Unable to determine Vebox Output Surface.");
    }

    return pSurface;
}

MOS_STATUS VPHAL_VEBOX_STATE_XE_XPM::VeboxSetHVSDNParams(
    PVPHAL_DENOISE_PARAMS pDNParams)
{
    MOS_STATUS               eStatus     = MOS_STATUS_UNKNOWN;
    PRENDERHAL_INTERFACE     pRenderHal  = nullptr;
    PVPHAL_VEBOX_STATE       pVeboxState = this;
    PVPHAL_VEBOX_RENDER_DATA pRenderData = nullptr;
    MhwVeboxInterfaceG12::MHW_VEBOX_CHROMA_PARAMS *veboxChromaParams = nullptr;

    pRenderHal  = pVeboxState->m_pRenderHal;
    pRenderData = GetLastExecRenderData();

    VPHAL_RENDER_CHK_NULL_RETURN(pDNParams);
    VPHAL_RENDER_CHK_NULL_RETURN(pRenderHal);
    VPHAL_RENDER_CHK_NULL_RETURN(pRenderData);

    // Populate the VEBOX DNDI parameters
    veboxChromaParams = (MhwVeboxInterfaceG12::MHW_VEBOX_CHROMA_PARAMS *)&pRenderData->VeboxChromaParams;

    if (nullptr == m_hvsDenoiser)
    {
        m_hvsDenoiser = MOS_New(VphalHVSDenoiserHpm, pRenderHal);
        if (m_hvsDenoiser)
        {
            m_hvsDenoiser->InitKernelParams(m_hvsKernelBinary, m_hvsKernelBinarySize);
        }
        else
        {
            VPHAL_RENDER_ASSERTMESSAGE("New VphalHVSDenoiser Failed!");
            eStatus = MOS_STATUS_NULL_POINTER;
            return eStatus;
        }
    }

    if (m_hvsDenoiser)
    {
        m_hvsDenoiser->Render(pDNParams);
        uint32_t *pHVSDenoiseParam = (uint32_t *)m_hvsDenoiser->GetDenoiseParams();
        if (pHVSDenoiseParam)
        {
            // Media kernel computed the HVS Denoise Parameters according to the specific mapping function.
            // Programming these Parameters to VEBOX for processing.
            VPHAL_RENDER_NORMALMESSAGE("Set HVS Denoised Parameters to VEBOX DNDI params");
            // DW0
            pRenderData->VeboxDNDIParams.dwDenoiseMaximumHistory = (pHVSDenoiseParam[0] & 0x000000ff);
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwDenoiseMaximumHistory %d", pRenderData->VeboxDNDIParams.dwDenoiseMaximumHistory);
            pRenderData->VeboxDNDIParams.dwDenoiseSTADThreshold = (pHVSDenoiseParam[0] & 0xfffe0000) >> 17;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwDenoiseSTADThreshold %d", pRenderData->VeboxDNDIParams.dwDenoiseSTADThreshold);
            // DW1
            pRenderData->VeboxDNDIParams.dwDenoiseASDThreshold = (pHVSDenoiseParam[1] & 0x00000fff);
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwDenoiseASDThreshold %d", pRenderData->VeboxDNDIParams.dwDenoiseASDThreshold);
            pRenderData->VeboxDNDIParams.dwDenoiseMPThreshold = (pHVSDenoiseParam[1] & 0x0f800000) >> 23;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwDenoiseMPThreshold %d", pRenderData->VeboxDNDIParams.dwDenoiseMPThreshold);
            pRenderData->VeboxDNDIParams.dwDenoiseHistoryDelta = (pHVSDenoiseParam[1] & 0xf0000000) >> 28;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwDenoiseHistoryDelta %d", pRenderData->VeboxDNDIParams.dwDenoiseHistoryDelta);
            // DW2
            pRenderData->VeboxDNDIParams.dwTDThreshold = (pHVSDenoiseParam[2] & 0xfff00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwTDThreshold %d", pRenderData->VeboxDNDIParams.dwTDThreshold);
            // DW3
            pRenderData->VeboxDNDIParams.dwLTDThreshold = (pHVSDenoiseParam[3] & 0xfff00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwLTDThreshold %d", pRenderData->VeboxDNDIParams.dwLTDThreshold);
            // DW4
            pRenderData->VeboxDNDIParams.dwDenoiseSCMThreshold = (pHVSDenoiseParam[4] & 0xfff00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwDenoiseSCMThreshold %d", pRenderData->VeboxDNDIParams.dwDenoiseSCMThreshold);
            // DW5
            pRenderData->VeboxDNDIParams.dwChromaSTADThreshold = (pHVSDenoiseParam[5] & 0xfffe0000) >> 17;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwChromaSTADThreshold %d", pRenderData->VeboxDNDIParams.dwChromaSTADThreshold);
            // DW6
            pRenderData->VeboxDNDIParams.dwChromaTDThreshold = (pHVSDenoiseParam[6] & 0xfff00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwChromaTDThreshold %d", pRenderData->VeboxDNDIParams.dwChromaTDThreshold);
            // DW7
            pRenderData->VeboxDNDIParams.dwChromaLTDThreshold = (pHVSDenoiseParam[7] & 0xfff00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwChromaLTDThreshold %d", pRenderData->VeboxDNDIParams.dwChromaLTDThreshold);
            // DW9
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[0] = (pHVSDenoiseParam[9] & 0x0000001f);
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeWeight[0] %d", pRenderData->VeboxDNDIParams.dwPixRangeWeight[0]);
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[1] = (pHVSDenoiseParam[9] & 0x000003e0) >> 5;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeWeight[1] %d", pRenderData->VeboxDNDIParams.dwPixRangeWeight[1]);
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[2] = (pHVSDenoiseParam[9] & 0x00007c00) >> 10;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeWeight[2] %d", pRenderData->VeboxDNDIParams.dwPixRangeWeight[2]);
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[3] = (pHVSDenoiseParam[9] & 0x000f8000) >> 15;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeWeight[3] %d", pRenderData->VeboxDNDIParams.dwPixRangeWeight[3]);
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[4] = (pHVSDenoiseParam[9] & 0x01f00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeWeight[4] %d", pRenderData->VeboxDNDIParams.dwPixRangeWeight[4]);
            pRenderData->VeboxDNDIParams.dwPixRangeWeight[5] = (pHVSDenoiseParam[9] & 0x3e000000) >> 25;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeWeight[5] %d", pRenderData->VeboxDNDIParams.dwPixRangeWeight[5]);
            // DW11
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[5] = (pHVSDenoiseParam[11] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeThreshold[5] %d", pRenderData->VeboxDNDIParams.dwPixRangeThreshold[5]);
            // DW12
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[4] = (pHVSDenoiseParam[12] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeThreshold[4] %d", pRenderData->VeboxDNDIParams.dwPixRangeThreshold[4]);
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[3] = (pHVSDenoiseParam[12] & 0x00001fff);
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeThreshold[3] %d", pRenderData->VeboxDNDIParams.dwPixRangeThreshold[3]);
            // DW13
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[2] = (pHVSDenoiseParam[13] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeThreshold[2] %d", pRenderData->VeboxDNDIParams.dwPixRangeThreshold[2]);
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[1] = (pHVSDenoiseParam[13] & 0x00001fff);
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeThreshold[1] %d", pRenderData->VeboxDNDIParams.dwPixRangeThreshold[1]);
            // DW14
            pRenderData->VeboxDNDIParams.dwPixRangeThreshold[0] = (pHVSDenoiseParam[14] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("HVS: pRenderData->VeboxDNDIParams.dwPixRangeThreshold[0] %d", pRenderData->VeboxDNDIParams.dwPixRangeThreshold[0]);
            // DW16
            veboxChromaParams->dwPixRangeWeightChromaU[0]    = (pHVSDenoiseParam[16] & 0x0000001f);
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaU[0] %d", veboxChromaParams->dwPixRangeWeightChromaU[0]);
            veboxChromaParams->dwPixRangeWeightChromaU[1]    = (pHVSDenoiseParam[16] & 0x000003e0) >> 5;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaU[1] %d", veboxChromaParams->dwPixRangeWeightChromaU[1]);
            veboxChromaParams->dwPixRangeWeightChromaU[2]    = (pHVSDenoiseParam[16] & 0x00007c00) >> 10;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaU[2] %d", veboxChromaParams->dwPixRangeWeightChromaU[2]);
            veboxChromaParams->dwPixRangeWeightChromaU[3]    = (pHVSDenoiseParam[16] & 0x000f8000) >> 15;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaU[3] %d", veboxChromaParams->dwPixRangeWeightChromaU[3]);
            veboxChromaParams->dwPixRangeWeightChromaU[4]    = (pHVSDenoiseParam[16] & 0x01f00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaU[4] %d", veboxChromaParams->dwPixRangeWeightChromaU[4]);
            veboxChromaParams->dwPixRangeWeightChromaU[5]    = (pHVSDenoiseParam[16] & 0x3e000000) >> 25;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaU[5] %d", veboxChromaParams->dwPixRangeWeightChromaU[5]);
            //DW18
            veboxChromaParams->dwPixRangeThresholdChromaU[5] = (pHVSDenoiseParam[18] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaU[5] %d", veboxChromaParams->dwPixRangeThresholdChromaU[5]);
            //DW19
            veboxChromaParams->dwPixRangeThresholdChromaU[4] = (pHVSDenoiseParam[19] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaU[4] %d", veboxChromaParams->dwPixRangeThresholdChromaU[4]);
            veboxChromaParams->dwPixRangeThresholdChromaU[3] = (pHVSDenoiseParam[19] & 0x00001fff);
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaU[3] %d", veboxChromaParams->dwPixRangeThresholdChromaU[3]);
            //DW20
            veboxChromaParams->dwPixRangeThresholdChromaU[2] = (pHVSDenoiseParam[20] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaU[2] %d", veboxChromaParams->dwPixRangeThresholdChromaU[2]);
            veboxChromaParams->dwPixRangeThresholdChromaU[1] = (pHVSDenoiseParam[20] & 0x00001fff);
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaU[1] %d", veboxChromaParams->dwPixRangeThresholdChromaU[1]);
            //DW21
            veboxChromaParams->dwPixRangeThresholdChromaU[0] = (pHVSDenoiseParam[21] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaU[0] %d", veboxChromaParams->dwPixRangeThresholdChromaU[0]);
            //DW23
            veboxChromaParams->dwPixRangeWeightChromaV[0] = (pHVSDenoiseParam[23] & 0x0000001f);
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaV[0] %d", veboxChromaParams->dwPixRangeWeightChromaV[0]);
            veboxChromaParams->dwPixRangeWeightChromaV[1] = (pHVSDenoiseParam[23] & 0x000003e0) >> 5;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaV[1] %d", veboxChromaParams->dwPixRangeWeightChromaV[1]);
            veboxChromaParams->dwPixRangeWeightChromaV[2] = (pHVSDenoiseParam[23] & 0x00007c00) >> 10;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaV[2] %d", veboxChromaParams->dwPixRangeWeightChromaV[2]);
            veboxChromaParams->dwPixRangeWeightChromaV[3] = (pHVSDenoiseParam[23] & 0x000f8000) >> 15;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaV[3] %d", veboxChromaParams->dwPixRangeWeightChromaV[3]);
            veboxChromaParams->dwPixRangeWeightChromaV[4] = (pHVSDenoiseParam[23] & 0x01f00000) >> 20;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaV[4] %d", veboxChromaParams->dwPixRangeWeightChromaV[4]);
            veboxChromaParams->dwPixRangeWeightChromaV[5] = (pHVSDenoiseParam[23] & 0x3e000000) >> 25;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeWeightChromaV[5] %d", veboxChromaParams->dwPixRangeWeightChromaV[5]);
            //DW25
            veboxChromaParams->dwPixRangeThresholdChromaV[5] = (pHVSDenoiseParam[25] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaV[5] %d", veboxChromaParams->dwPixRangeThresholdChromaV[5]);
            //DW26
            veboxChromaParams->dwPixRangeThresholdChromaV[4] = (pHVSDenoiseParam[26] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaV[4] %d", veboxChromaParams->dwPixRangeThresholdChromaV[4]);
            veboxChromaParams->dwPixRangeThresholdChromaV[3] = (pHVSDenoiseParam[26] & 0x00001fff);
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaV[3] %d", veboxChromaParams->dwPixRangeThresholdChromaV[3]);
            //DW27
            veboxChromaParams->dwPixRangeThresholdChromaV[2] = (pHVSDenoiseParam[27] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaV[2] %d", veboxChromaParams->dwPixRangeThresholdChromaV[2]);
            veboxChromaParams->dwPixRangeThresholdChromaV[1] = (pHVSDenoiseParam[27] & 0x00001fff);
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaV[1] %d", veboxChromaParams->dwPixRangeThresholdChromaV[1]);
            //DW28
            veboxChromaParams->dwPixRangeThresholdChromaV[0] = (pHVSDenoiseParam[28] & 0x1fff0000) >> 16;
            VPHAL_RENDER_NORMALMESSAGE("veboxChromaParams->dwPixRangeThresholdChromaV[0] %d", veboxChromaParams->dwPixRangeThresholdChromaV[0]);
            eStatus = MOS_STATUS_SUCCESS;
        }
    }

    return eStatus;
}
