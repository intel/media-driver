/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     vp_vebox_cmd_packet.cpp
//! \brief    vebox packet which used in by mediapipline.
//! \details  vebox packet provide the structures and generate the cmd buffer which mediapipline will used.
//!

#include "vp_vebox_cmd_packet.h"
#include "vp_utils.h"
#include "mos_resource_defs.h"
#include "hal_oca_interface.h"
#include "vp_render_sfc_m12.h"
#include "vp_render_ief.h"
#include "vp_feature_caps.h"
#include "vp_platform_interface.h"
#include "mhw_vebox_itf.h"
#include "mhw_mi_itf.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_utilities.h"
#include "hal_oca_interface.h"

namespace vp {
MOS_STATUS VpVeboxCmdPacket::RenderVeboxCmd(
    MOS_COMMAND_BUFFER                      *CmdBuffer,
    MHW_VEBOX_DI_IECP_CMD_PARAMS            &VeboxDiIecpCmdParams,
    VPHAL_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
    MHW_VEBOX_STATE_CMD_PARAMS              &VeboxStateCmdParams,
    MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
    PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams)
{
    VP_FUNC_CALL();

    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE  pRenderHal;
    PMOS_INTERFACE        pOsInterface;
    PMHW_MI_INTERFACE     pMhwMiInterface;
    PMHW_VEBOX_INTERFACE  pVeboxInterface;
    bool                  bDiVarianceEnable;
    const MHW_VEBOX_HEAP *pVeboxHeap     = nullptr;
    VpVeboxRenderData *   pRenderData    = GetLastExecRenderData();
    MediaPerfProfiler *   pPerfProfiler  = nullptr;
    MOS_CONTEXT *         pOsContext     = nullptr;
    PMHW_MI_MMIOREGISTERS pMmioRegisters = nullptr;
    MOS_COMMAND_BUFFER    CmdBufferInUse;
    PMOS_COMMAND_BUFFER   pCmdBufferInUse = nullptr;
    uint32_t              curPipe         = 0;
    uint8_t               inputPipe       = 0;
    uint32_t              numPipe         = 1;
    bool                  bMultipipe      = false;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_veboxInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface->pOsContext);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_mhwMiInterface->GetMmioRegisters());
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);

    pRenderHal      = m_hwInterface->m_renderHal;
    pMhwMiInterface = m_hwInterface->m_mhwMiInterface;
    pOsInterface    = m_hwInterface->m_osInterface;
    pVeboxInterface = m_hwInterface->m_veboxInterface;
    pPerfProfiler   = pRenderHal->pPerfProfiler;
    pOsContext      = m_hwInterface->m_osInterface->pOsContext;
    pMmioRegisters  = pMhwMiInterface->GetMmioRegisters();
    pCmdBufferInUse = CmdBuffer;

    auto scalability = GetMediaScalability();
    m_veboxItf = std::static_pointer_cast<mhw::vebox::Itf>(pVeboxInterface->GetNewVeboxInterface());

    if (m_veboxItf)
    {
        VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&pVeboxHeap));
    }
    else
    {
        VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->GetVeboxHeapInfo(&pVeboxHeap));
    }
    VP_RENDER_CHK_NULL_RETURN(pVeboxHeap);

    m_miItf = std::static_pointer_cast<mhw::mi::Itf>(m_hwInterface->m_mhwMiInterface->GetNewMiInterface());

#ifdef _MMC_SUPPORTED

    VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->setVeboxPrologCmd(pMhwMiInterface, CmdBuffer));

#endif

    bDiVarianceEnable = m_PacketCaps.bDI;

    SetupSurfaceStates(
        &VeboxSurfaceStateCmdParams);

    SetupVeboxState(
        &VeboxStateCmdParams);

    VP_RENDER_CHK_STATUS_RETURN(SetupDiIecpState(
        bDiVarianceEnable,
        &VeboxDiIecpCmdParams));

    VP_RENDER_CHK_STATUS_RETURN(IsCmdParamsValid(
        VeboxStateCmdParams,
        VeboxDiIecpCmdParams,
        VeboxSurfaceStateCmdParams));

    // Initialize command buffer and insert prolog
    VP_RENDER_CHK_STATUS_RETURN(InitCmdBufferWithVeParams(pRenderHal, *CmdBuffer, pGenericPrologParams));

    //---------------------------------
    // Initialize Vebox Surface State Params
    //---------------------------------
    VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceStateCmdParams(
        &VeboxSurfaceStateCmdParams, &MhwVeboxSurfaceStateCmdParams));

    for (curPipe = 0; curPipe < numPipe; curPipe++)
    {
        if (bMultipipe)
        {
            // initialize the command buffer struct
            MOS_ZeroMemory(&CmdBufferInUse, sizeof(MOS_COMMAND_BUFFER));
            bool frameTrackingRequested = m_PacketCaps.lastSubmission && (numPipe - 1 == curPipe);
            scalability->SetCurrentPipeIndex((uint8_t)curPipe);
            scalability->GetCmdBuffer(&CmdBufferInUse, frameTrackingRequested);
            pCmdBufferInUse = &CmdBufferInUse;
        }
        else
        {
            pCmdBufferInUse = CmdBuffer;
        }
        pVeboxInterface->SetVeboxIndex(curPipe, numPipe, m_IsSfcUsed);

        HalOcaInterface::On1stLevelBBStart(*pCmdBufferInUse, *pOsContext, pOsInterface->CurrentGpuContextHandle, *pMhwMiInterface, *pMmioRegisters);

        char ocaMsg[] = "VP APG Vebox Packet";
        HalOcaInterface::TraceMessage(*pCmdBufferInUse, *pOsContext, ocaMsg, sizeof(ocaMsg));

        HalOcaInterface::TraceOcaSkuValue(*pCmdBufferInUse, *pOsInterface);

        // Add vphal param to log.
        HalOcaInterface::DumpVphalParam(*pCmdBufferInUse, *pOsContext, pRenderHal->pVphalOcaDumper);

        VP_RENDER_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectStartCmd((void *)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StartPredicate(pRenderHal->pMhwMiInterface, pCmdBufferInUse));

        // Add compressible info of input/output surface to log
        if (this->m_currentSurface && VeboxSurfaceStateCmdParams.pSurfOutput)
        {
            std::string info   = "in_comps = " + std::to_string(int(this->m_currentSurface->osSurface->bCompressible)) + ", out_comps = " + std::to_string(int(VeboxSurfaceStateCmdParams.pSurfOutput->osSurface->bCompressible));
            const char *ocaLog = info.c_str();
            HalOcaInterface::TraceMessage(*pCmdBufferInUse, *pOsContext, ocaLog, info.size());
        }

        if (bMultipipe)
        {
            // Insert prolog with VE params
#ifdef _MMC_SUPPORTED

            VP_RENDER_CHK_STATUS_RETURN(pVeboxInterface->setVeboxPrologCmd(pMhwMiInterface, pCmdBufferInUse));

#endif

            MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
            MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
            genericPrologParams.pOsInterface  = pRenderHal->pOsInterface;
            genericPrologParams.pvMiInterface = pRenderHal->pMhwMiInterface;
            genericPrologParams.bMmcEnabled   = pGenericPrologParams ? pGenericPrologParams->bMmcEnabled : false;
            VP_RENDER_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmd(pCmdBufferInUse, &genericPrologParams));

            VP_RENDER_CHK_STATUS_RETURN(scalability->SyncPipe(syncAllPipes, 0, pCmdBufferInUse));

            // Enable Watchdog Timer
            if (m_miItf)
            {
                VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddWatchdogTimerStartCmd(pCmdBufferInUse));
            }
            else
            {
                VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddWatchdogTimerStartCmd(pCmdBufferInUse));
            }

#if (_DEBUG || _RELEASE_INTERNAL)
            // Add noop for simu no output issue
            if (curPipe > 0)
            {
                if (m_miItf)
                {
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                }
                else
                {
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                    pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                }
                if (m_IsSfcUsed)
                {
                    if (m_miItf)
                    {
                        SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                        SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                        SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                        SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    }
                    else
                    {
                        pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                        pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                        pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                        pMhwMiInterface->AddMiNoop(pCmdBufferInUse, nullptr);
                    }
                }
            }
#endif
        }

        //---------------------------------
        // Send CMD: Vebox_State
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxState(
            pVeboxInterface,
            pCmdBufferInUse,
            &VeboxStateCmdParams,
            0));

        //---------------------------------
        // Send CMD: Vebox_Surface_State
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxSurfaces(
            pVeboxInterface,
            pCmdBufferInUse,
            &MhwVeboxSurfaceStateCmdParams));

        //---------------------------------
        // Send CMD: SFC pipe commands
        //---------------------------------
        if (m_IsSfcUsed)
        {
            VP_RENDER_CHK_NULL_RETURN(m_sfcRender);

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SetSfcPipe(curPipe, numPipe));

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SetupSfcState(m_renderTarget));

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SendSfcCmd(
                (pRenderData->DI.bDeinterlace || pRenderData->DN.bDnEnabled),
                pCmdBufferInUse));
        }

        HalOcaInterface::OnDispatch(*pCmdBufferInUse, *pOsContext, *pMhwMiInterface, *pMmioRegisters);

        //---------------------------------
        // Send CMD: Vebox_DI_IECP
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxDiIecp(
            pVeboxInterface,
            pCmdBufferInUse,
            &VeboxDiIecpCmdParams));

        VP_RENDER_CHK_NULL_RETURN(pOsInterface);
        VP_RENDER_CHK_NULL_RETURN(pOsInterface->pfnGetSkuTable);
        auto *skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            // Add PPC fulsh
            if (m_miItf)
            {
                auto &params = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
                params       = {};
                params.bEnablePPCFlush = true;
                m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse);
            }
            else
            {
                MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
                FlushDwParams.bEnablePPCFlush = true;
                VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(pCmdBufferInUse, &FlushDwParams));
            }
        }

        if (bMultipipe)
        {
            // MI FlushDw, for vebox output green block issue
            if (m_miItf)
            {
                auto &params = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
                params       = {};
                m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse);
            }
            else
            {
                MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
                VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(pCmdBufferInUse, &FlushDwParams));
            }

            VP_RENDER_CHK_STATUS_RETURN(scalability->SyncPipe(syncAllPipes, 0, pCmdBufferInUse));
        }

        //---------------------------------
        // Write GPU Status Tag for Tag based synchronization
        //---------------------------------
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            VP_RENDER_CHK_STATUS_RETURN(SendVecsStatusTag(
                pMhwMiInterface,
                pOsInterface,
                pCmdBufferInUse));
        }

        //---------------------------------
        // Write Sync tag for Vebox Heap Synchronization
        // If KMD frame tracking is on, the synchronization of Vebox Heap will use Status tag which
        // is updated using KMD frame tracking.
        //---------------------------------
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            if (m_miItf)
            {
                auto &params            = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
                params                  = {};
                params.pOsResource      = (PMOS_RESOURCE)&pVeboxHeap->DriverResource;
                params.dwResourceOffset = pVeboxHeap->uiOffsetSync;
                params.dwDataDW1        = pVeboxHeap->dwNextTag;
                m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse);
            }
            else
            {
                MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
                FlushDwParams.pOsResource      = (PMOS_RESOURCE)&pVeboxHeap->DriverResource;
                FlushDwParams.dwResourceOffset = pVeboxHeap->uiOffsetSync;
                FlushDwParams.dwDataDW1        = pVeboxHeap->dwNextTag;

                VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(
                    pCmdBufferInUse,
                    &FlushDwParams));
            }
        }

        if (bMultipipe)
        {
            // Disable Watchdog Timer
            if (m_miItf)
            {
                VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddWatchdogTimerStopCmd(pCmdBufferInUse));
            }
            else
            {
                VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddWatchdogTimerStopCmd(pCmdBufferInUse));
            }
        }

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StopPredicate(pRenderHal->pMhwMiInterface, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectEndCmd((void *)pRenderHal, pOsInterface, pRenderHal->pMhwMiInterface, pCmdBufferInUse));

        HalOcaInterface::On1stLevelBBEnd(*pCmdBufferInUse, *pOsInterface);

        if (pOsInterface->bNoParsingAssistanceInKmd)
        {
            if (m_miItf)
            {
                m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr);
            }
            else
            {
                VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(
                    pCmdBufferInUse,
                    nullptr));
            }
        }
        else if (RndrCommonIsMiBBEndNeeded(pOsInterface))
        {
            // Add Batch Buffer end command (HW/OS dependent)
            if (m_miItf)
            {
                m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr);
            }
            else
            {
                VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiBatchBufferEnd(
                    pCmdBufferInUse,
                    nullptr));
            }
        }

        if (bMultipipe)
        {
            scalability->ReturnCmdBuffer(pCmdBufferInUse);
        }
    }

    if (bMultipipe)
    {
        scalability->SetCurrentPipeIndex(inputPipe);
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE_ID, true, m_hwInterface->m_osInterface->pOsContext);
    }
    else
    {
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE_ID, false, m_hwInterface->m_osInterface->pOsContext);
    }

    MT_LOG2(MT_VP_HAL_RENDER_VE, MT_NORMAL, MT_VP_MHW_VE_SCALABILITY_EN, bMultipipe, MT_VP_MHW_VE_SCALABILITY_USE_SFC, m_IsSfcUsed);

    return eStatus;
}  // namespace vp

MOS_STATUS VpVeboxCmdPacket::SendVecsStatusTag(
    PMHW_MI_INTERFACE                   pMhwMiInterface,
    PMOS_INTERFACE                      pOsInterface,
    PMOS_COMMAND_BUFFER                 pCmdBuffer)
{
    VP_FUNC_CALL();

    PMOS_RESOURCE                       gpuStatusBuffer = nullptr;
    //std::shared_ptr<mhw::mi::Itf>       m_miItf         = nullptr;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------------
    VP_RENDER_CHK_NULL_RETURN(pMhwMiInterface);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(pCmdBuffer);

    // Get GPU Status buffer
    pOsInterface->pfnGetGpuStatusBufferResource(pOsInterface, gpuStatusBuffer);
    VP_RENDER_CHK_NULL_RETURN(gpuStatusBuffer);

    // Register the buffer
    VP_RENDER_CHK_STATUS_RETURN(pOsInterface->pfnRegisterResource(
                                  pOsInterface,
                                  gpuStatusBuffer,
                                  true,
                                  true));

    m_miItf = std::static_pointer_cast<mhw::mi::Itf>(pMhwMiInterface->GetNewMiInterface());

    if (m_miItf)
    {
        //SETPAR_AND_ADDCMD(MI_FLUSH_DW, m_miItf, pCmdBuffer);
        auto &params             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        params                   = {};
        params.pOsResource       = gpuStatusBuffer;;
        params.dwResourceOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
        params.dwDataDW1         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
        m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer);

    }
    else
    {
        MOS_ZeroMemory(&FlushDwParams, sizeof(FlushDwParams));
        FlushDwParams.pOsResource       = gpuStatusBuffer;
        FlushDwParams.dwResourceOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
        FlushDwParams.dwDataDW1         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

        VP_RENDER_CHK_STATUS_RETURN(pMhwMiInterface->AddMiFlushDwCmd(
                                      pCmdBuffer,
                                      &FlushDwParams));
    }

    // Increase buffer tag for next usage
    pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

    return eStatus;
}
}
