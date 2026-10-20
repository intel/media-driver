/*
* Copyright (c) 2018-2023, Intel Corporation
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
#include "hal_oca_interface_next.h"
#include "vp_render_ief.h"
#include "vp_feature_caps.h"
#include "vp_platform_interface.h"
#include "vp_hal_ddi_utils.h"
#include "mhw_vebox_itf.h"
#include "mhw_mi_itf.h"
#include "mhw_mi_cmdpar.h"
#include "mhw_utilities.h"
#include "media_scalability_defs.h"
#include "renderhal_platform_interface_next.h"

namespace vp {

#define INTERP(x0, x1, x, y0, y1)   ((uint32_t) floor(y0+(x-x0)*(y1-y0)/(double)(x1-x0)))

const uint32_t  VpVeboxCmdPacket::m_satP1Table[MHW_STE_FACTOR_MAX + 1] = {
    0x00000000, 0xfffffffe, 0xfffffffc, 0xfffffffa, 0xfffffff6, 0xfffffff4, 0xfffffff2, 0xfffffff0, 0xffffffee, 0xffffffec };

const uint32_t   VpVeboxCmdPacket::m_satS0Table[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ef, 0x00000100, 0x00000113, 0x00000129, 0x0000017a, 0x000001a2, 0x000001d3, 0x00000211, 0x00000262, 0x000002d1 };

const uint32_t   VpVeboxCmdPacket::m_satS1Table[MHW_STE_FACTOR_MAX + 1] = {
    0x000000ab, 0x00000080, 0x00000066, 0x00000055, 0x000000c2, 0x000000b9, 0x000000b0, 0x000000a9, 0x000000a2, 0x0000009c };

void VpVeboxCmdPacket::UpdateCpPrepareResources()
{
    VP_FUNC_CALL();

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(pRenderData);
    // For 3DLut usage, it update in CpPrepareResources() for kernel usage, should
    // reupdate here. For other feature usage, it already update in vp_pipeline
    if (pRenderData->HDR3DLUT.is3DLutTableUpdatedByKernel == true)
    {
        VP_RENDER_NORMALMESSAGE("Update CP Prepare Resource for 3DLut kernel.");
        PMOS_RESOURCE source[VPHAL_MAX_SOURCES] = {nullptr};
        PMOS_RESOURCE target[VPHAL_MAX_TARGETS] = {nullptr};

        if ((nullptr != m_hwInterface->m_osInterface) &&
            (nullptr != m_hwInterface->m_osInterface->osCpInterface))
        {
            VP_SURFACE *surf = GetSurface(SurfaceTypeVeboxInput);
            VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(surf);
            source[0] = &(surf->osSurface->OsResource);

            VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(m_renderTarget);
            target[0] = &(m_renderTarget->osSurface->OsResource);

            m_hwInterface->m_osInterface->osCpInterface->PrepareResources((void **)source, 1, (void **)target, 1);
        }
    }
}

MOS_STATUS VpVeboxCmdPacket::InitCmdBufferWithVeParams(
    PRENDERHAL_INTERFACE pRenderHal,
    MOS_COMMAND_BUFFER & CmdBuffer,
    PRENDERHAL_GENERIC_PROLOG_PARAMS pGenericPrologParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                              eStatus = MOS_STATUS_SUCCESS;
    RENDERHAL_GENERIC_PROLOG_PARAMS_NEXT    genericPrologParams = {};

    genericPrologParams.bEnableMediaFrameTracking      = pGenericPrologParams->bEnableMediaFrameTracking;
    genericPrologParams.bMmcEnabled                    = pGenericPrologParams->bMmcEnabled;
    genericPrologParams.dwMediaFrameTrackingAddrOffset = pGenericPrologParams->dwMediaFrameTrackingAddrOffset;
    genericPrologParams.dwMediaFrameTrackingTag        = pGenericPrologParams->dwMediaFrameTrackingTag;
    genericPrologParams.presMediaFrameTrackingSurface  = pGenericPrologParams->presMediaFrameTrackingSurface;

    genericPrologParams.VEngineHintParams.BatchBufferCount = 2;
    //genericPrologParams.VEngineHintParams.resScalableBatchBufs[0] = CmdBuffer[0].OsResource;
    //genericPrologParams.VEngineHintParams.resScalableBatchBufs[1] = CmdBuffer[1].OsResource;
    genericPrologParams.VEngineHintParams.UsingFrameSplit = true;
    genericPrologParams.VEngineHintParams.UsingSFC = false;
    genericPrologParams.VEngineHintParams.EngineInstance[0] = 0;
    genericPrologParams.VEngineHintParams.EngineInstance[1] = 1;
    genericPrologParams.VEngineHintParams.NeedSyncWithPrevious = true;
    genericPrologParams.VEngineHintParams.SameEngineAsLastSubmission = true;

    pRenderHal->pOsInterface->VEEnable = false;

    // Initialize command buffer and insert prolog
    VP_RENDER_CHK_STATUS_RETURN(pRenderHal->pfnInitCommandBuffer(
        pRenderHal,
        &CmdBuffer,
        (PRENDERHAL_GENERIC_PROLOG_PARAMS)&genericPrologParams));

    return eStatus;
}

bool VpVeboxCmdPacket::IsFormatMMCSupported(MOS_FORMAT Format)
{
    VP_FUNC_CALL();

    // Check if Sample Format is supported
    if ((Format != Format_YUY2) &&
        (Format != Format_Y210) &&
        (Format != Format_Y410) &&
        (Format != Format_Y216) &&
        (Format != Format_Y416) &&
        (Format != Format_P010) &&
        (Format != Format_P016) &&
        (Format != Format_AYUV) &&
        (Format != Format_NV21) &&
        (Format != Format_NV12) &&
        (Format != Format_UYVY) &&
        (Format != Format_YUYV) &&
        (Format != Format_R10G10B10A2)   &&
        (Format != Format_B10G10R10A2)   &&
        (Format != Format_A8B8G8R8)      &&
        (Format != Format_X8B8G8R8)      &&
        (Format != Format_A8R8G8B8)      &&
        (Format != Format_X8R8G8B8)      &&
        (Format != Format_A16B16G16R16F) &&
        (Format != Format_A16R16G16B16F))
    {
      VP_RENDER_NORMALMESSAGE("Unsupported Format '0x%08x' for VEBOX MMC ouput.", Format);
      return false;
    }

    return true;
}

MOS_STATUS VpVeboxCmdPacket::SetSfcMmcParams()
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_sfcRender);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_mmc);

    // Decompress resource if surfaces need write from a un-align offset
    if ((m_renderTarget->osSurface->CompressionMode != MOS_MMC_DISABLED) && m_sfcRender->IsSFCUncompressedWriteNeeded(m_renderTarget))
    {
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
        MOS_SURFACE details = {};

        eStatus = m_osInterface->pfnGetResourceInfo(m_osInterface, &m_renderTarget->osSurface->OsResource, &details);

        if (eStatus != MOS_STATUS_SUCCESS)
        {
            VP_RENDER_ASSERTMESSAGE("Get SFC target surface resource info failed.");
        }

        if (!m_renderTarget->osSurface->OsResource.bUncompressedWriteNeeded)
        {
            eStatus = m_osInterface->pfnDecompResource(m_osInterface, &m_renderTarget->osSurface->OsResource);

            if (eStatus != MOS_STATUS_SUCCESS)
            {
                VP_RENDER_ASSERTMESSAGE("inplace decompression failed for sfc target.");
            }
            else
            {
                VP_RENDER_NORMALMESSAGE("inplace decompression enabled for sfc target RECT is not compression block align.");
                m_renderTarget->osSurface->OsResource.bUncompressedWriteNeeded = 1;
            }
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->SetMmcParams(m_renderTarget->osSurface,
                                                        IsFormatMMCSupported(m_renderTarget->osSurface->Format),
                                                        m_mmc->IsMmcEnabled()));

    return MOS_STATUS_SUCCESS;
}

VP_SURFACE *VpVeboxCmdPacket::GetSurface(SurfaceType type)
{
    VP_FUNC_CALL();

    auto it = m_surfSetting.surfGroup.find(type);
    VP_SURFACE *surf = (m_surfSetting.surfGroup.end() != it) ? it->second : nullptr;
    if (SurfaceTypeVeboxCurrentOutput == type && nullptr == surf && !m_IsSfcUsed)
    {
        // Vebox output case.
        surf = m_renderTarget;
    }
    else if (SurfaceTypeVeboxInput == type && surf)
    {
        // The vp surface object for external surface will be destroyed by hw filter before packet submit.
        // Store the vp surface object inside packet.
        if (MOS_FAILED(m_allocator->CopyVpSurface(*m_currentSurface ,*surf)))
        {
            return nullptr;
        }
        m_currentSurface->rcMaxSrc = m_currentSurface->rcSrc;
        surf = m_currentSurface;
    }
    else if (SurfaceTypeVeboxPreviousInput == type && surf)
    {
        // The vp surface object for external surface will be destroyed by hw filter before packet submit.
        // Store the vp surface object inside packet.
        if (MOS_FAILED(m_allocator->CopyVpSurface(*m_previousSurface ,*surf)))
        {
            return nullptr;
        }
        surf = m_previousSurface;
    }
    return surf;
}

bool VpVeboxCmdPacket::UseKernelResource()
{
    VP_FUNC_CALL();

    return false;
}

MOS_STATUS VpVeboxCmdPacket::SendVeboxCmd(MOS_COMMAND_BUFFER* commandBuffer)
{
    VP_FUNC_CALL();

    MOS_STATUS                              eStatus;
    int32_t                                 iRemaining;
    VP_VEBOX_SURFACE_STATE_CMD_PARAMS    VeboxSurfaceStateCmdParams;
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      MhwVeboxSurfaceStateCmdParams;
    MHW_MI_FLUSH_DW_PARAMS                  FlushDwParams;
    RENDERHAL_GENERIC_PROLOG_PARAMS         GenericPrologParams;

    eStatus                 = MOS_STATUS_SUCCESS;
    iRemaining              = 0;

    VP_RENDER_CHK_NULL_RETURN(commandBuffer);

    eStatus = PrepareVeboxCmd(
                  commandBuffer,
                  GenericPrologParams,
                  iRemaining);

    if (eStatus != MOS_STATUS_SUCCESS)
    {
        CmdErrorHanlde(commandBuffer, iRemaining);
    }
    else
    {
        eStatus = RenderVeboxCmd(
                      commandBuffer,
                      VeboxSurfaceStateCmdParams,
                      MhwVeboxSurfaceStateCmdParams,
                      FlushDwParams,
                      &GenericPrologParams);
        if (eStatus != MOS_STATUS_SUCCESS)
        {
          // Failed -> discard all changes in Command Buffer
            CmdErrorHanlde(commandBuffer, iRemaining);
        }
    }

    return eStatus;
}

void VpVeboxCmdPacket::CmdErrorHanlde(
    MOS_COMMAND_BUFFER  *CmdBuffer,
    int32_t             &iRemaining)
{
    VP_FUNC_CALL();

    int32_t     i= 0;

    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(CmdBuffer);
    // Buffer overflow - display overflow size
    if (CmdBuffer->iRemaining < 0)
    {
        VP_RENDER_ASSERTMESSAGE("Command Buffer overflow by %d bytes", CmdBuffer->iRemaining);
    }

    // Move command buffer back to beginning
    i = iRemaining - CmdBuffer->iRemaining;
    CmdBuffer->iRemaining = iRemaining;
    CmdBuffer->iOffset -= i;
    CmdBuffer->pCmdPtr = CmdBuffer->pCmdBase + CmdBuffer->iOffset / sizeof(uint32_t);
}

MOS_STATUS VpVeboxCmdPacket::PrepareVeboxCmd(
    MOS_COMMAND_BUFFER*                      CmdBuffer,
    RENDERHAL_GENERIC_PROLOG_PARAMS&         GenericPrologParams,
    int32_t&                                 iRemaining)
{
    VP_FUNC_CALL();

    MOS_STATUS                              eStatus      = MOS_STATUS_SUCCESS;
    PMOS_INTERFACE                          pOsInterface = m_hwInterface->m_osInterface;
    VpVeboxRenderData                       *pRenderData  = GetLastExecRenderData();
    PMOS_RESOURCE                           gpuStatusBuffer = nullptr;

    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(pOsInterface);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface->osSurface);
    VP_RENDER_CHK_NULL_RETURN(pRenderData);

    // Set initial state
    iRemaining = CmdBuffer->iRemaining;

    //---------------------------
    // Set Performance Tags
    //---------------------------
    VP_RENDER_CHK_STATUS_RETURN(VeboxSetPerfTag());
    pOsInterface->pfnResetPerfBufferID(pOsInterface);
    pOsInterface->pfnSetPerfTag(pOsInterface, pRenderData->PerfTag);

    MOS_ZeroMemory(&GenericPrologParams, sizeof(GenericPrologParams));

    VP_RENDER_CHK_STATUS_RETURN(SetMediaFrameTracking(GenericPrologParams));

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::RenderVeboxCmd(
    MOS_COMMAND_BUFFER                      *CmdBuffer,
    VP_VEBOX_SURFACE_STATE_CMD_PARAMS    &VeboxSurfaceStateCmdParams,
    MHW_VEBOX_SURFACE_STATE_CMD_PARAMS      &MhwVeboxSurfaceStateCmdParams,
    MHW_MI_FLUSH_DW_PARAMS                  &FlushDwParams,
    PRENDERHAL_GENERIC_PROLOG_PARAMS        pGenericPrologParams)
{
    VP_FUNC_CALL();

    MOS_STATUS            eStatus = MOS_STATUS_SUCCESS;
    PRENDERHAL_INTERFACE  pRenderHal;
    PMOS_INTERFACE        pOsInterface;
    bool                  bDiVarianceEnable;
    const MHW_VEBOX_HEAP *pVeboxHeap     = nullptr;
    VpVeboxRenderData *   pRenderData    = GetLastExecRenderData();
    MOS_CONTEXT *         pOsContext     = nullptr;
    PMHW_MI_MMIOREGISTERS pMmioRegisters = nullptr;
    MOS_COMMAND_BUFFER    CmdBufferInUse;
    PMOS_COMMAND_BUFFER   pCmdBufferInUse = nullptr;
    uint32_t              curPipe         = 0;
    uint8_t               inputPipe       = 0;
    uint32_t              numPipe         = 1;
    bool                  bMultipipe      = false;

    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_renderHal);
    VP_RENDER_CHK_NULL_RETURN(m_miItf);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_osInterface->pOsContext);
    VP_RENDER_CHK_NULL_RETURN(m_miItf->GetMmioRegisters());
    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(CmdBuffer);
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);

    pRenderHal      = m_hwInterface->m_renderHal;
    pOsInterface    = m_hwInterface->m_osInterface;
    pOsContext      = m_hwInterface->m_osInterface->pOsContext;
    pMmioRegisters  = m_miItf->GetMmioRegisters();
    pCmdBufferInUse = CmdBuffer;

    auto report      = (VpFeatureReport *)(m_hwInterface->m_reporting);
    auto scalability = GetMediaScalability();
    VP_RENDER_CHK_NULL_RETURN(m_veboxItf);
    mhw::vebox::VEBOX_STATE_PAR& veboxStateCmdParams = m_veboxItf->MHW_GETPAR_F(VEBOX_STATE)();
    mhw::vebox::VEB_DI_IECP_PAR& veboxDiIecpCmdParams = m_veboxItf->MHW_GETPAR_F(VEB_DI_IECP)();

    VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&pVeboxHeap));
    VP_RENDER_CHK_NULL_RETURN(pVeboxHeap);

    VP_RENDER_CHK_STATUS_RETURN(SetVeboxProCmd(CmdBuffer));

    // Initialize the scalability
    curPipe    = scalability->GetCurrentPipe();
    inputPipe  = (uint8_t)curPipe;
    numPipe    = scalability->GetPipeNumber();
    bMultipipe = (numPipe > 1) ? true : false;

    VP_RENDER_CHK_STATUS_RETURN(SetVeboxIndex(0, numPipe, m_IsSfcUsed));

    bDiVarianceEnable = m_PacketCaps.bDI;

    VP_RENDER_CHK_STATUS_RETURN(SetupSurfaceStates(
        &VeboxSurfaceStateCmdParams));

    VP_RENDER_CHK_STATUS_RETURN(SetupVeboxState(veboxStateCmdParams));

    VP_RENDER_CHK_STATUS_RETURN(SetupDiIecpState(
        bDiVarianceEnable,
        veboxDiIecpCmdParams));

    VP_RENDER_CHK_STATUS_RETURN(IsCmdParamsValid(
        veboxStateCmdParams,
        veboxDiIecpCmdParams,
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

        VP_RENDER_CHK_STATUS_RETURN(SetVeboxIndex(curPipe, numPipe, m_IsSfcUsed));

        AddCommonOcaMessage(pCmdBufferInUse, (MOS_CONTEXT_HANDLE)pOsContext, pOsInterface, pRenderHal, pMmioRegisters);

        VP_RENDER_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddPerfCollectStartCmd(pRenderHal, pOsInterface, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StartPredicateNext(pOsInterface, m_miItf, pCmdBufferInUse));
#if (_DEBUG || _RELEASE_INTERNAL)
        VP_RENDER_CHK_STATUS_RETURN(StoreCSEngineIdRegMem(pCmdBufferInUse, pVeboxHeap));
#endif
        // Add compressible info of input/output surface to log
        if (this->m_currentSurface && VeboxSurfaceStateCmdParams.pSurfOutput)
        {
            std::string info   = "in_comps = " + std::to_string(int(this->m_currentSurface->osSurface->bCompressible)) + ", out_comps = " + std::to_string(int(VeboxSurfaceStateCmdParams.pSurfOutput->osSurface->bCompressible));
            const char *ocaLog = info.c_str();
            HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, (MOS_CONTEXT_HANDLE)pOsContext, ocaLog, info.size());
        }

        if (bMultipipe)
        {
            // Insert prolog with VE params

            VP_RENDER_CHK_STATUS_RETURN(SetVeboxProCmd(pCmdBufferInUse));

            MHW_GENERIC_PROLOG_PARAMS genericPrologParams;
            MOS_ZeroMemory(&genericPrologParams, sizeof(genericPrologParams));
            genericPrologParams.pOsInterface  = pRenderHal->pOsInterface;
            genericPrologParams.bMmcEnabled   = pGenericPrologParams ? pGenericPrologParams->bMmcEnabled : false;
            VP_RENDER_CHK_STATUS_RETURN(Mhw_SendGenericPrologCmdNext(pCmdBufferInUse, &genericPrologParams, m_miItf));

            VP_RENDER_CHK_STATUS_RETURN(scalability->SyncPipe(syncAllPipes, 0, pCmdBufferInUse));

            // Enable Watchdog Timer
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddWatchdogTimerStartCmd(pCmdBufferInUse));

#if (_DEBUG || _RELEASE_INTERNAL)
            // Add noop for simu no output issue
            if (curPipe > 0)
            {
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                if (m_IsSfcUsed)
                {
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                    SETPAR_AND_ADDCMD(MI_NOOP, m_miItf, pCmdBufferInUse);
                }
            }
#endif
        }

        //---------------------------------
        // Send CMD: Vebox_State
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxState(
            pCmdBufferInUse));

        //---------------------------------
        // Send CMD: Vebox_Surface_State
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxSurfaces(
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

            bool enableVeboxOutputSurf = false;
            if (m_vpUserFeatureControl)
            {
                enableVeboxOutputSurf = m_vpUserFeatureControl->IsVeboxOutputSurfEnabled();
            }

            VP_RENDER_CHK_STATUS_RETURN(m_sfcRender->SendSfcCmd(
                (enableVeboxOutputSurf || pRenderData->DI.bDeinterlace || pRenderData->DN.bDnEnabled),
                pCmdBufferInUse));
        }

        pRenderHal->pRenderHalPltInterface->OnDispatch(pRenderHal, pCmdBufferInUse, pOsInterface, pMmioRegisters);
        //HalOcaInterfaceNext::OnDispatch(*pCmdBufferInUse, *pOsContext, m_miItf, *pMmioRegisters);

        //---------------------------------
        // Send CMD: Vebox_DI_IECP
        //---------------------------------
        VP_RENDER_CHK_STATUS_RETURN(SetVeboxDiIecp(
            pCmdBufferInUse));

        VP_RENDER_CHK_NULL_RETURN(pOsInterface);
        VP_RENDER_CHK_NULL_RETURN(pOsInterface->pfnGetSkuTable);
        auto *skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            // Add PPC fulsh
            auto &miFlushDwParams           = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            miFlushDwParams                 = {};
            miFlushDwParams.bEnablePPCFlush = true;
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse));
        }

        if (bMultipipe)
        {
            // MI FlushDw, for vebox output green block issue
            auto &params             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            params                   = {};
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse));

            VP_RENDER_CHK_STATUS_RETURN(scalability->SyncPipe(syncAllPipes, 0, pCmdBufferInUse));
        }

        if (m_PacketCaps.enableSFCLinearOutputByTileConvert)
        {
            VP_RENDER_CHK_STATUS_RETURN(AddTileConvertStates(CmdBuffer, MhwVeboxSurfaceStateCmdParams));
            report->GetFeatures().sfcLinearOutputByTileConvert = true;
        }

        //---------------------------------
        // Write GPU Status Tag for Tag based synchronization
        //---------------------------------
#if !EMUL
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            VP_RENDER_CHK_STATUS_RETURN(SendVecsStatusTag(
                pOsInterface,
                pCmdBufferInUse));
        }
#endif

        //---------------------------------
        // Write Sync tag for Vebox Heap Synchronization
        // If KMD frame tracking is on, the synchronization of Vebox Heap will use Status tag which
        // is updated using KMD frame tracking.
        //---------------------------------
        if (!pOsInterface->bEnableKmdMediaFrameTracking)
        {
            auto &params             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
            params                   = {};
            params.pOsResource       = (PMOS_RESOURCE)&pVeboxHeap->DriverResource;
            params.dwResourceOffset  = pVeboxHeap->uiOffsetSync;
            params.dwDataDW1         = pVeboxHeap->dwNextTag;
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBufferInUse));
        }

        if (bMultipipe)
        {
            // Disable Watchdog Timer
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddWatchdogTimerStopCmd(pCmdBufferInUse));
        }

        VP_RENDER_CHK_STATUS_RETURN(NullHW::StopPredicateNext(pOsInterface, m_miItf, pCmdBufferInUse));

        VP_RENDER_CHK_STATUS_RETURN(pRenderHal->pRenderHalPltInterface->AddPerfCollectEndCmd(pRenderHal, pOsInterface, pCmdBufferInUse));

#if (_DEBUG || _RELEASE_INTERNAL)
        VP_RENDER_CHK_STATUS_RETURN(StallBatchBuffer(pCmdBufferInUse));
#endif

        HalOcaInterfaceNext::On1stLevelBBEnd(*pCmdBufferInUse, *pOsInterface);

        if (pOsInterface->bNoParsingAssistanceInKmd)
        {
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr));
        }
        else if (RndrCommonIsMiBBEndNeeded(pOsInterface))
        {
            // Add Batch Buffer end command (HW/OS dependent)
            VP_RENDER_CHK_STATUS_RETURN(m_miItf->AddMiBatchBufferEnd(pCmdBufferInUse, nullptr));
        }

        if (bMultipipe)
        {
            VP_RENDER_CHK_STATUS_RETURN(scalability->ReturnCmdBuffer(pCmdBufferInUse));
        }
    }

    if (bMultipipe)
    {
        scalability->SetCurrentPipeIndex(inputPipe);
    }

    report->GetFeatures().VeboxScalability  = bMultipipe;

    MT_LOG2(MT_VP_HAL_RENDER_VE, MT_NORMAL, MT_VP_MHW_VE_SCALABILITY_EN, bMultipipe, MT_VP_MHW_VE_SCALABILITY_USE_SFC, m_IsSfcUsed);

    return eStatus;
}
// Meida copy has the same logic, when Format_R10G10B10A2/Format_B10G10R10A2, the output is AYUV, and in this WA there is corruption for these two format.
MOS_FORMAT VpVeboxCmdPacket::AdjustFormatForTileConvert(MOS_FORMAT format)
{
    if (format == Format_R10G10B10A2 ||
        format == Format_B10G10R10A2 ||
        format == Format_Y410 ||
        format == Format_Y210)
    {
        // RGB10 not supported without IECP. Re-map RGB10/RGB10 as AYUV
        // Y410/Y210 has HW issue. Remap to AYUV.
        return Format_AYUV;
    }
    else if (format == Format_A8)
    {
        return Format_P8;
    }
    else
    {
        return format;
    }
}

void VpVeboxCmdPacket::AddCommonOcaMessage(PMOS_COMMAND_BUFFER pCmdBufferInUse, MOS_CONTEXT_HANDLE pOsContext, PMOS_INTERFACE pOsInterface, PRENDERHAL_INTERFACE pRenderHal, PMHW_MI_MMIOREGISTERS pMmioRegisters)
{
    VP_FUNC_CALL();

    HalOcaInterfaceNext::On1stLevelBBStart(*pCmdBufferInUse, pOsContext, pOsInterface->CurrentGpuContextHandle, m_miItf, *pMmioRegisters);

    char ocaMsg[] = "VP APG Vebox Packet";
    HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, pOsContext, ocaMsg, sizeof(ocaMsg));

    VpVeboxRenderData *pRenderData = GetLastExecRenderData();
    if (pRenderData)
    {
        MHW_VEBOX_IECP_PARAMS IecpParams = pRenderData->GetIECPParams();
        if (pRenderData->IECP.STE.bStdEnabled && IecpParams.ColorPipeParams.StdParams.param)
        {
            char ocaMsg_std[] = "Customized STD state is used";
            HalOcaInterfaceNext::TraceMessage(*pCmdBufferInUse, pOsContext, ocaMsg_std, sizeof(ocaMsg_std));
        }
    }

    HalOcaInterfaceNext::TraceOcaSkuValue(*pCmdBufferInUse, *pOsInterface);

    // Add vphal param to log.
    HalOcaInterfaceNext::DumpVphalParam(*pCmdBufferInUse, pOsContext, pRenderHal->pVphalOcaDumper);

    if (m_vpUserFeatureControl)
    {
        HalOcaInterfaceNext::DumpVpUserFeautreControlInfo(*pCmdBufferInUse, pOsContext, m_vpUserFeatureControl->GetOcaFeautreControlInfo());
    }

}

MOS_STATUS VpVeboxCmdPacket::InitVeboxSurfaceStateCmdParams(
    PVP_VEBOX_SURFACE_STATE_CMD_PARAMS    pVpHalVeboxSurfaceStateCmdParams,
    PMHW_VEBOX_SURFACE_STATE_CMD_PARAMS      pMhwVeboxSurfaceStateCmdParams)
{
    VP_FUNC_CALL();

    MOS_STATUS                       eStatus = MOS_STATUS_SUCCESS;

    VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams);
    VP_RENDER_CHK_NULL_RETURN(pMhwVeboxSurfaceStateCmdParams);

    MOS_ZeroMemory(pMhwVeboxSurfaceStateCmdParams, sizeof(*pMhwVeboxSurfaceStateCmdParams));

    pMhwVeboxSurfaceStateCmdParams->bDIEnable       = pVpHalVeboxSurfaceStateCmdParams->bDIEnable;
    pMhwVeboxSurfaceStateCmdParams->b3DlutEnable    = pVpHalVeboxSurfaceStateCmdParams->b3DlutEnable;

    if (pVpHalVeboxSurfaceStateCmdParams->pSurfInput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfInput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfInput));
        pMhwVeboxSurfaceStateCmdParams->SurfInput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface->YPlaneOffset.iYOffset;
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_INPUT, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfInput->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface);
        pMhwVeboxSurfaceStateCmdParams->bOutputValid = true;
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfOutput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfOutput));
        pMhwVeboxSurfaceStateCmdParams->SurfOutput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->YPlaneOffset.iYOffset;
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_OUT, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfOutput->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfSTMM));
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_STMM, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfSTMM->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfDNOutput));
        pMhwVeboxSurfaceStateCmdParams->SurfDNOutput.dwYoffset = pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface->YPlaneOffset.iYOffset;
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_DNOUT, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfDNOutput->osSurface->Format);
    }
    if (pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput)
    {
        VP_RENDER_CHK_NULL_RETURN(pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput->osSurface);
        VP_RENDER_CHK_STATUS_RETURN(InitVeboxSurfaceParams(
                                      pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput,
                                      &pMhwVeboxSurfaceStateCmdParams->SurfSkinScoreOutput));
        MT_LOG2(MT_VP_MHW_VE_SURFSTATE_SKINSCORE, MT_NORMAL, MT_SURF_TILE_MODE, pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput->osSurface->TileModeGMM,
            MT_SURF_MOS_FORMAT, pVpHalVeboxSurfaceStateCmdParams->pSurfSkinScoreOutput->osSurface->Format);
    }

    if (m_inputDepth)
    {
        pMhwVeboxSurfaceStateCmdParams->SurfInput.dwBitDepth = m_inputDepth;
    }

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SendVecsStatusTag(
    PMOS_INTERFACE                      pOsInterface,
    PMOS_COMMAND_BUFFER                 pCmdBuffer)
{
    VP_FUNC_CALL();

    PMOS_RESOURCE                       gpuStatusBuffer = nullptr;
    MHW_MI_FLUSH_DW_PARAMS              FlushDwParams;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;

    //------------------------------------
    VP_RENDER_CHK_NULL_RETURN(m_miItf);
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

    VP_RENDER_CHK_NULL_RETURN(m_miItf);

    auto &params             = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    params                   = {};
    params.pOsResource       = gpuStatusBuffer;;
    params.dwResourceOffset  = pOsInterface->pfnGetGpuStatusTagOffset(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    params.dwDataDW1         = pOsInterface->pfnGetGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);
    VP_RENDER_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(pCmdBuffer));

    // Increase buffer tag for next usage
    pOsInterface->pfnIncrementGpuStatusTag(pOsInterface, MOS_GPU_CONTEXT_VEBOX);

    return eStatus;
}

bool VpVeboxCmdPacket::RndrCommonIsMiBBEndNeeded(
    PMOS_INTERFACE           pOsInterface)
{
    VP_FUNC_CALL();

    bool needed = false;

    if (nullptr == pOsInterface)
        return false;

    return needed;
}

MOS_STATUS VpVeboxCmdPacket::InitSfcRender()
{
    VP_FUNC_CALL();

    if (nullptr == m_sfcRender)
    {
        VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
        VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_vpPlatformInterface);
        VP_RENDER_CHK_STATUS_RETURN(m_hwInterface->m_vpPlatformInterface->CreateSfcRender(
            m_sfcRender,
            *m_hwInterface,
            m_allocator));
        VP_RENDER_CHK_NULL_RETURN(m_sfcRender);
    }
    VP_PUBLIC_CHK_STATUS_RETURN(m_sfcRender->Init());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::Init()
{
    VP_FUNC_CALL();

    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface);
    VP_RENDER_CHK_NULL_RETURN(m_hwInterface->m_skuTable);

    VP_RENDER_CHK_STATUS_RETURN(InitSfcRender());

    if (nullptr == m_currentSurface)
    {
        m_currentSurface = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_currentSurface);
    }
    else
    {
        m_currentSurface->Clean();
    }

    if (nullptr == m_previousSurface)
    {
        m_previousSurface = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_previousSurface);
    }
    else
    {
        m_previousSurface->Clean();
    }

    if (nullptr == m_renderTarget)
    {
        m_renderTarget = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_renderTarget);
    }
    else
    {
        m_renderTarget->Clean();
    }

    if (nullptr == m_originalOutput)
    {
        m_originalOutput = m_allocator->AllocateVpSurface();
        VP_CHK_SPACE_NULL_RETURN(m_originalOutput);
    }
    else
    {
        m_originalOutput->Clean();
    }

    MOS_ZeroMemory(&m_veboxPacketSurface, sizeof(VEBOX_PACKET_SURFACE_PARAMS));
    m_surfSetting.Clean();

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::Prepare()
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::PrepareState()
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (m_packetResourcesPrepared)
    {
        VP_RENDER_NORMALMESSAGE("Resource Prepared, skip this time");
        return MOS_STATUS_SUCCESS;
    }

    VP_RENDER_CHK_STATUS_RETURN(SetupIndirectStates());

    VP_RENDER_CHK_STATUS_RETURN(UpdateVeboxStates());

    if (m_veboxItf)
    {
        const MHW_VEBOX_HEAP *veboxHeap = nullptr;

        VP_RENDER_CHK_STATUS_RETURN(m_veboxItf->GetVeboxHeapInfo(&veboxHeap));
        VP_RENDER_CHK_NULL_RETURN(veboxHeap);

        m_veboxHeapCurState = veboxHeap->uiCurState;
    }

    m_packetResourcesPrepared = true;

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::SetUpdatedExecuteResource(
    VP_SURFACE                          *inputSurface,
    VP_SURFACE                          *outputSurface,
    VP_SURFACE                          *previousSurface,
    VP_SURFACE_SETTING                  &surfSetting)
{
    VP_FUNC_CALL();
    VP_RENDER_CHK_NULL_RETURN(inputSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface);
    VP_RENDER_CHK_NULL_RETURN(inputSurface->osSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface->osSurface);
    m_allocator->UpdateResourceUsageType(&inputSurface->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
    m_allocator->UpdateResourceUsageType(&outputSurface->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);

    // Set current src = current primary input
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_renderTarget ,*outputSurface));

    // Init packet surface params.
    m_surfSetting                                   = surfSetting;
    m_veboxPacketSurface.pCurrInput                 = GetSurface(SurfaceTypeVeboxInput);
    m_veboxPacketSurface.pStatisticsOutput          = GetSurface(SurfaceTypeStatistics);
    m_veboxPacketSurface.pCurrOutput                = GetSurface(SurfaceTypeVeboxCurrentOutput);
    m_veboxPacketSurface.pPrevInput                 = GetSurface(SurfaceTypeVeboxPreviousInput);
    m_veboxPacketSurface.pSTMMInput                 = GetSurface(SurfaceTypeSTMMIn);
    m_veboxPacketSurface.pSTMMOutput                = GetSurface(SurfaceTypeSTMMOut);
    m_veboxPacketSurface.pDenoisedCurrOutput        = GetSurface(SurfaceTypeDNOutput);
    m_veboxPacketSurface.pPrevOutput                = GetSurface(SurfaceTypeVeboxPreviousOutput);
    m_veboxPacketSurface.pAlphaOrVignette           = GetSurface(SurfaceTypeAlphaOrVignette);
    m_veboxPacketSurface.pLaceOrAceOrRgbHistogram   = GetSurface(SurfaceTypeLaceAceRGBHistogram);
    m_veboxPacketSurface.pSurfSkinScoreOutput       = GetSurface(SurfaceTypeSkinScore);

    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrInput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pStatisticsOutput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pLaceOrAceOrRgbHistogram);

    // Adjust boundary for statistics surface block
    VP_RENDER_CHK_STATUS_RETURN(AdjustBlockStatistics());

    if (m_PacketCaps.bSFC)
    {
        VP_RENDER_CHK_STATUS_RETURN(SetSfcMmcParams());
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::PacketInit(
    VP_SURFACE                          *inputSurface,
    VP_SURFACE                          *outputSurface,
    VP_SURFACE                          *previousSurface,
    VP_SURFACE_SETTING                  &surfSetting,
    VP_EXECUTE_CAPS                     packetCaps)
{
    VP_FUNC_CALL();

    VpVeboxRenderData       *pRenderData = GetLastExecRenderData();
    m_packetResourcesPrepared = false;

    VP_RENDER_CHK_NULL_RETURN(pRenderData);
    VP_RENDER_CHK_NULL_RETURN(inputSurface);
    VP_RENDER_CHK_NULL_RETURN(outputSurface);
    VP_RENDER_CHK_STATUS_RETURN(pRenderData->Init());

    m_PacketCaps      = packetCaps;
    VP_RENDER_NORMALMESSAGE("m_PacketCaps %x", m_PacketCaps.value);

    VP_RENDER_CHK_STATUS_RETURN(Init());
    VP_RENDER_CHK_NULL_RETURN(m_allocator);
    VP_RENDER_CHK_NULL_RETURN(m_currentSurface);
    VP_RENDER_CHK_NULL_RETURN(m_renderTarget);
    VP_RENDER_CHK_NULL_RETURN(m_previousSurface);

    VP_RENDER_CHK_STATUS_RETURN(InitSurfMemCacheControl(packetCaps));

    m_IsSfcUsed = packetCaps.bSFC;

    //update VEBOX resource GMM resource usage type
    m_allocator->UpdateResourceUsageType(&inputSurface->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_INPUT_PICTURE_FF);
    m_allocator->UpdateResourceUsageType(&outputSurface->osSurface->OsResource, MOS_HW_RESOURCE_USAGE_VP_OUTPUT_PICTURE_FF);

    // Init packet surface params.
    m_surfSetting                                   = surfSetting;
    m_veboxPacketSurface.pCurrInput                 = GetSurface(SurfaceTypeVeboxInput);
    m_veboxPacketSurface.pStatisticsOutput          = GetSurface(SurfaceTypeStatistics);
    m_veboxPacketSurface.pCurrOutput                = GetSurface(SurfaceTypeVeboxCurrentOutput);
    m_veboxPacketSurface.pPrevInput                 = GetSurface(SurfaceTypeVeboxPreviousInput);
    m_veboxPacketSurface.pSTMMInput                 = GetSurface(SurfaceTypeSTMMIn);
    m_veboxPacketSurface.pSTMMOutput                = GetSurface(SurfaceTypeSTMMOut);
    m_veboxPacketSurface.pDenoisedCurrOutput        = GetSurface(SurfaceTypeDNOutput);
    m_veboxPacketSurface.pPrevOutput                = GetSurface(SurfaceTypeVeboxPreviousOutput);
    m_veboxPacketSurface.pAlphaOrVignette           = GetSurface(SurfaceTypeAlphaOrVignette);
    m_veboxPacketSurface.pLaceOrAceOrRgbHistogram   = GetSurface(SurfaceTypeLaceAceRGBHistogram);
    m_veboxPacketSurface.pSurfSkinScoreOutput       = GetSurface(SurfaceTypeSkinScore);
    m_veboxPacketSurface.pInnerTileConvertInput     = GetSurface(SurfaceTypeInnerTileConvertInput);
    // Set current src = current primary input
    if (m_veboxPacketSurface.pInnerTileConvertInput)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_renderTarget, *m_veboxPacketSurface.pInnerTileConvertInput));

        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_originalOutput, *outputSurface));
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->CopyVpSurface(*m_renderTarget, *outputSurface));
    }

    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pCurrInput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pStatisticsOutput);
    VP_RENDER_CHK_NULL_RETURN(m_veboxPacketSurface.pLaceOrAceOrRgbHistogram);

    m_DNDIFirstFrame    = (!m_PacketCaps.bRefValid && (m_PacketCaps.bDN || m_PacketCaps.bDI));
    m_DIOutputFrames    = MEDIA_VEBOX_DI_OUTPUT_CURRENT;

    auto curInput = m_veboxPacketSurface.pCurrInput;
    auto curOutput = m_veboxPacketSurface.pCurrOutput;
    if (!m_IsSfcUsed &&
        ((uint32_t)curInput->rcSrc.bottom < curInput->osSurface->dwHeight ||
        (uint32_t)curInput->rcSrc.right < curInput->osSurface->dwWidth))
    {
        curInput->bVEBOXCroppingUsed = true;
        VP_RENDER_NORMALMESSAGE("bVEBOXCroppingUsed = true, input: rcSrc.bottom: %d, rcSrc.right: %d; dwHeight: %d, dwHeight: %d;",
            (uint32_t)curInput->rcSrc.bottom,
            (uint32_t)curInput->rcSrc.right,
            curInput->osSurface->dwHeight,
            curInput->osSurface->dwWidth);

        if (curOutput)
        {
            curOutput->bVEBOXCroppingUsed = true;
            VP_RENDER_NORMALMESSAGE("                           output: rcSrc.bottom: %d, rcSrc.right: %d; dwHeight: %d, dwHeight: %d;",
                (uint32_t)curOutput->rcSrc.bottom,
                (uint32_t)curOutput->rcSrc.right,
                curOutput->osSurface->dwHeight,
                curOutput->osSurface->dwWidth);
        }
    }
    else
    {
        curInput->bVEBOXCroppingUsed = false;
        if (curOutput)
        {
            curOutput->bVEBOXCroppingUsed = false;
        }
    }

    if (!m_PacketCaps.bSFC)
    {
        // Reset ForceInputHeight8AlignedFlag for bSFC == false.
        // For bSFC == true, it's configured in VpScalingFilter::CalculateEngineParams.
        if (m_veboxItf)
        {
            m_veboxItf->SetForceInputHeight8AlignedFlag(false);
        }  
    }

    // Adjust boundary for statistics surface block
    VP_RENDER_CHK_STATUS_RETURN(AdjustBlockStatistics());

    // Get Vebox Secure mode form policy
    m_useKernelResource = packetCaps.bSecureVebox;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::Submit(MOS_COMMAND_BUFFER* commandBuffer, uint8_t packetPhase)
{
    VP_FUNC_CALL();

    MOS_STATUS    eStatus = MOS_STATUS_SUCCESS;
    VpVeboxRenderData   *pRenderData = GetLastExecRenderData();

    // use the StateIndex which was saved in PrepareState() to resolve the 2pass sfc mismatch issue
    if (m_veboxItf)
    {
        m_veboxItf->SetVeboxHeapStateIndex(m_veboxHeapCurState);
    }

    if (m_currentSurface && m_currentSurface->osSurface)
    {
        // Ensure the input is ready to be read
        // Currently, mos RegisterResourcere cannot sync the 3d resource.
        // Temporaly, call sync resource to do the sync explicitly.
        // Sync need be done after switching context.
#if MOS_MEDIASOLO_SUPPORTED
        if (!m_hwInterface->m_osInterface->bSoloInUse)
#endif
        {
            m_allocator->SyncOnResource(
                &m_currentSurface->osSurface->OsResource,
                false);
        }
    }

    // Send vebox command
    VP_RENDER_CHK_STATUS_RETURN(SendVeboxCmd(commandBuffer));

#if (_DEBUG || _RELEASE_INTERNAL)
    // Debug interface with state heap check
    VP_RENDER_CHK_STATUS_RETURN(DumpVeboxStateHeap())
#endif

    return eStatus;
}

void VpVeboxCmdPacket::CopySurfaceValue(
    VP_SURFACE                  *pTargetSurface,
    VP_SURFACE                  *pSourceSurface)
{
    VP_FUNC_CALL();

    if (pTargetSurface == nullptr)
    {
        VP_RENDER_ASSERTMESSAGE("Input pTargetSurface is null");
        return;
    }
    *pTargetSurface = *pSourceSurface;
}

VpVeboxCmdPacket::VpVeboxCmdPacket(
    MediaTask * task,
    PVP_MHWINTERFACE hwInterface,
    PVpAllocator &allocator,
    VPMediaMemComp *mmc) :
    CmdPacket(task),
    VpCmdPacket(task, hwInterface, allocator, mmc, VP_PIPELINE_PACKET_VEBOX),
    VpVeboxCmdPacketBase(task, hwInterface, allocator, mmc)
{
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(hwInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(hwInterface->m_vpPlatformInterface);
    m_veboxItf = hwInterface->m_vpPlatformInterface->GetMhwVeboxItf();
    m_miItf = hwInterface->m_vpPlatformInterface->GetMhwMiItf();
    m_vpUserFeatureControl = hwInterface->m_userFeatureControl;
}

VpVeboxCmdPacket:: ~VpVeboxCmdPacket()
{
    VP_FUNC_CALL();

    MOS_Delete(m_sfcRender);
    MOS_Delete(m_lastExecRenderData);
    MOS_Delete(m_surfMemCacheCtl);

    m_allocator->DestroyVpSurface(m_currentSurface);
    m_allocator->DestroyVpSurface(m_previousSurface);
    m_allocator->DestroyVpSurface(m_renderTarget);
    m_allocator->DestroyVpSurface(m_originalOutput);
}

bool VpVeboxCmdPacket::IsVeboxGamutStateNeeded()
{
    VP_FUNC_CALL();

    VpVeboxRenderData *renderData = GetLastExecRenderData();

    if (renderData)
    {
        return renderData->HDR3DLUT.bHdr3DLut || renderData->IECP.CGC.bCGCEnabled;
    }
    else
    {
        return false;
    }
}

MOS_STATUS VpVeboxCmdPacket::VeboxSetPerfTag()
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_PERFTAG              pPerfTag = nullptr;
    VpVeboxRenderData           *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface);
    VP_PUBLIC_CHK_NULL_RETURN(m_currentSurface->osSurface);

    MOS_FORMAT srcFmt = m_currentSurface->osSurface->Format;

    pPerfTag = &pRenderData->PerfTag;

    switch (srcFmt)
    {
        case Format_NV12:
            return VeboxSetPerfTagNv12();

        CASE_PA_FORMAT:
            return VeboxSetPerfTagPaFormat();

        case Format_P010:
            // P010 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P010;
            break;

        case Format_P016:
            // P016 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P016;
            break;

        case Format_P210:
            // P210 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P210;
            break;

        case Format_P216:
            // P216 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_P216;
            break;

        case Format_Y210:
            // Y210 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y210;
            break;

        case Format_Y216:
            // Y216 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y216;
            break;

        case Format_Y410:
            // Y410 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y410;
            break;

        case Format_Y416:
            // Y416 Input Support for VEBOX, SFC
            *pPerfTag = VPHAL_VEBOX_Y416;
            break;

        CASE_RGB32_FORMAT:
            *pPerfTag = VPHAL_VEBOX_RGB32;
            break;

        case Format_AYUV:
            *pPerfTag = VPHAL_VEBOX_AYUV;
            break;

        case Format_A16B16G16R16:
        case Format_A16R16G16B16:
        case Format_A16B16G16R16F:
        case Format_A16R16G16B16F:
            *pPerfTag = VPHAL_VEBOX_RGB64;
            break;

        default:
            VP_RENDER_ASSERTMESSAGE("Format Not found.");
            *pPerfTag = VPHAL_NONE;
            eStatus = MOS_STATUS_INVALID_PARAMETER;
    } // switch (srcFmt)

    return eStatus;
}

MOS_STATUS VpVeboxCmdPacket::VeboxSetPerfTagNv12()
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_PERFTAG              pPerfTag = nullptr;
    VpVeboxRenderData           *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget->osSurface);

    MOS_FORMAT dstFormat = m_renderTarget->osSurface->Format;

    pPerfTag = &pRenderData->PerfTag;

    if (pRenderData->IsDiEnabled())
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_NV12_DNDI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_NV12_DNDI_PA;
            }
        }
        else
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PL_DI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PL_DI_PA;
            }
        }
    }
    else
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_NV12_DN_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_NV12_DN_422CP;
                        break;
                    case Format_RGB32:
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                        *pPerfTag = VPHAL_NV12_DN_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_PUBLIC_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_NV12_DN_420CP;
            }
            else
            {
                *pPerfTag = VPHAL_NV12_DN_NV12;
            }
        }
        else
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_NV12_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_NV12_422CP;
                        break;
                    case Format_RGB32:
                        *pPerfTag = VPHAL_NV12_RGB32CP;
                        break;
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                    case Format_R10G10B10A2:
                    case Format_B10G10R10A2:
                        *pPerfTag = VPHAL_NV12_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_RENDER_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                *pPerfTag = VPHAL_NV12_420CP;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpVeboxCmdPacket::VeboxSetPerfTagPaFormat()
{
    VP_FUNC_CALL();

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    PVPHAL_PERFTAG              pPerfTag = nullptr;
    VpVeboxRenderData           *pRenderData = GetLastExecRenderData();

    VP_PUBLIC_CHK_NULL_RETURN(pRenderData);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget);
    VP_PUBLIC_CHK_NULL_RETURN(m_renderTarget->osSurface);

    MOS_FORMAT dstFormat = m_renderTarget->osSurface->Format;

    pPerfTag = &pRenderData->PerfTag;

    if (pRenderData->IsDiEnabled())
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PA_DNDI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PA_DNDI_PA;
            }
        }
        else
        {
            if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PA_DI_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PA_DI_PA;
            }
        }
    }
    else
    {
        if (pRenderData->DN.bDnEnabled ||
            pRenderData->DN.bChromaDnEnabled)
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_PA_DN_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_PA_DN_422CP;
                        break;
                    case Format_RGB32:
                        *pPerfTag = VPHAL_PA_DN_RGB32CP;
                        break;
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                    case Format_R10G10B10A2:
                    case Format_B10G10R10A2:
                        *pPerfTag = VPHAL_PA_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_RENDER_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else if (IsIECPEnabled())
            {
                *pPerfTag = VPHAL_PA_DN_422CP;
            }
            else
            {
                *pPerfTag = VPHAL_PA_DN_PA;
            }
        }
        else
        {
            if (IsOutputPipeVebox())
            {
                switch (dstFormat)
                {
                    case Format_NV12:
                        *pPerfTag = VPHAL_PA_420CP;
                        break;
                    CASE_PA_FORMAT:
                        *pPerfTag = VPHAL_PA_422CP;
                        break;
                    case Format_RGB32:
                        *pPerfTag = VPHAL_PA_RGB32CP;
                        break;
                    case Format_A8R8G8B8:
                    case Format_A8B8G8R8:
                    case Format_R10G10B10A2:
                    case Format_B10G10R10A2:
                        *pPerfTag = VPHAL_PA_RGB32CP;
                        break;
                    case Format_P010:
                    case Format_P016:
                    case Format_Y410:
                    case Format_Y416:
                    case Format_Y210:
                    case Format_Y216:
                    case Format_AYUV:
                    case Format_Y8:
                    case Format_Y16S:
                    case Format_Y16U:
                    case Format_A16B16G16R16F:
                    case Format_A16R16G16B16F:
                        *pPerfTag = VPHAL_NONE;
                        break;
                    default:
                        VP_RENDER_ASSERTMESSAGE("Output Format Not found.");
                        return MOS_STATUS_INVALID_PARAMETER;
                }
            }
            else
            {
                *pPerfTag = VPHAL_PA_422CP;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MHW_CSPACE VpVeboxCmdPacket::VpHalCspace2MhwCspace(VPHAL_CSPACE cspace)
{
    VP_FUNC_CALL();

    switch (cspace)
    {
        case CSpace_Source:
            return MHW_CSpace_Source;

        case CSpace_RGB:
            return MHW_CSpace_RGB;

        case CSpace_YUV:
            return MHW_CSpace_YUV;

        case CSpace_Gray:
            return MHW_CSpace_Gray;

        case CSpace_Any:
            return MHW_CSpace_Any;

        case CSpace_sRGB:
            return MHW_CSpace_sRGB;

        case CSpace_stRGB:
            return MHW_CSpace_stRGB;

        case CSpace_BT601:
            return MHW_CSpace_BT601;

        case CSpace_BT601_FullRange:
            return MHW_CSpace_BT601_FullRange;

        case CSpace_BT709:
            return MHW_CSpace_BT709;

        case CSpace_BT709_FullRange:
            return MHW_CSpace_BT709_FullRange;

        case CSpace_xvYCC601:
            return MHW_CSpace_xvYCC601;

        case CSpace_xvYCC709:
            return MHW_CSpace_xvYCC709;

        case CSpace_BT601Gray:
            return MHW_CSpace_BT601Gray;

        case CSpace_BT601Gray_FullRange:
            return MHW_CSpace_BT601Gray_FullRange;

        case CSpace_BT2020:
            return MHW_CSpace_BT2020;

        case CSpace_BT2020_RGB:
            return MHW_CSpace_BT2020_RGB;

        case CSpace_BT2020_FullRange:
            return MHW_CSpace_BT2020_FullRange;

        case CSpace_BT2020_stRGB:
            return MHW_CSpace_BT2020_stRGB;

        case CSpace_None:
        default:
            return MHW_CSpace_None;
    }
}
}