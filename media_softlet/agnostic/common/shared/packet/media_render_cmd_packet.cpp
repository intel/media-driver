/*
* Copyright (c) 2020-2021, Intel Corporation
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
#include "renderhal_platform_interface.h"
#include "hal_oca_interface.h"
#include "mos_interface.h"

#define COMPUTE_WALKER_THREAD_SPACE_WIDTH 1
#define COMPUTE_WALKER_THREAD_SPACE_HEIGHT 1
#define COMPUTE_WALKER_THREAD_SPACE_DEPTH 1

RenderCmdPacket::RenderCmdPacket(MediaTask* task, PMOS_INTERFACE pOsinterface, RENDERHAL_INTERFACE *renderHal)
    : CmdPacket(task),
      RenderCmdPacketNext(task, pOsinterface, renderHal)
{
}

RenderCmdPacket::~RenderCmdPacket()
{
    Destroy();
}

MOS_STATUS RenderCmdPacket::AddPipeControl(PMHW_MI_INTERFACE mhwMiInterface, MOS_COMMAND_BUFFER* commandBuffer)
{
    PMOS_INTERFACE    pOsInterface = nullptr;
    RENDER_PACKET_CHK_NULL_RETURN(mhwMiInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pOsInterface);

    pOsInterface = m_renderHal->pOsInterface;
    if (m_miItf)
    {
        auto& params = m_miItf->MHW_GETPAR_F(PIPE_CONTROL)();
        params                               = {};
        params.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
        params.bGenericMediaStateClear       = true;
        params.bIndirectStatePointersDisable = true;
        params.bDisableCSStall               = false;

        RENDER_PACKET_CHK_NULL_RETURN(pOsInterface->pfnGetSkuTable);
        auto *skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            // Add PPC fulsh
            params.bPPCFlush = true;
        }

        m_miItf->MHW_ADDCMD_F(PIPE_CONTROL)(commandBuffer);
    }
    else
    {
        MHW_PIPE_CONTROL_PARAMS pipeControlParams;

        MOS_ZeroMemory(&pipeControlParams, sizeof(pipeControlParams));
        pipeControlParams.dwFlushMode                   = MHW_FLUSH_WRITE_CACHE;
        pipeControlParams.bGenericMediaStateClear       = true;
        pipeControlParams.bIndirectStatePointersDisable = true;
        pipeControlParams.bDisableCSStall               = false;

        RENDER_PACKET_CHK_NULL_RETURN(pOsInterface->pfnGetSkuTable);
        auto *skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            // Add PPC fulsh
            pipeControlParams.bPPCFlush = true;
        }

        RENDER_PACKET_CHK_STATUS_RETURN(mhwMiInterface->AddPipeControl(commandBuffer, nullptr, &pipeControlParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::MediaStateFlush(PMHW_MI_INTERFACE mhwMiInterface, MOS_COMMAND_BUFFER* commandBuffer, MHW_MEDIA_STATE_FLUSH_PARAM flushParam)
{
    RENDER_PACKET_CHK_NULL_RETURN(mhwMiInterface);

    if (m_miItf)
    {
        auto& params = m_miItf->MHW_GETPAR_F(MEDIA_STATE_FLUSH)();
        params                              = {};
        params.ui8InterfaceDescriptorOffset = flushParam.ui8InterfaceDescriptorOffset;
        params.bFlushToGo                   = flushParam.bFlushToGo;
        m_miItf->MHW_ADDCMD_F(MEDIA_STATE_FLUSH)(commandBuffer);
    }
    else
    {
        RENDER_PACKET_CHK_STATUS_RETURN(mhwMiInterface->AddMediaStateFlush(commandBuffer, nullptr, &flushParam));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCmdPacket::AddMiBatchBufferEnd(PMHW_MI_INTERFACE mhwMiInterface, PMOS_COMMAND_BUFFER commandBuffer, PMHW_BATCH_BUFFER batchBuffer)
{
    RENDER_PACKET_CHK_NULL_RETURN(mhwMiInterface);

    if (m_miItf)
    {
        m_miItf->AddMiBatchBufferEnd(commandBuffer, nullptr);
    }
    else
    {
        RENDER_PACKET_CHK_STATUS_RETURN(mhwMiInterface->AddMiBatchBufferEnd(commandBuffer, nullptr));
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
    MhwRenderInterface* pMhwRender = nullptr;
    MHW_MEDIA_STATE_FLUSH_PARAM         FlushParam = {};
    bool                                bEnableSLM = false;
    RENDERHAL_GENERIC_PROLOG_PARAMS     GenericPrologParams = {};
    MOS_RESOURCE                        GpuStatusBuffer = {};
    MediaPerfProfiler* pPerfProfiler = nullptr;
    MOS_CONTEXT* pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS               pMmioRegisters = nullptr;

    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pMhwRenderInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pMhwMiInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pMhwRenderInterface->GetMmioRegisters());
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pOsInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pOsInterface->pOsContext);
    RENDER_PACKET_CHK_NULL_RETURN(commandBuffer);

    eStatus = MOS_STATUS_UNKNOWN;
    pOsInterface = m_renderHal->pOsInterface;
    pMhwMiInterface = m_renderHal->pMhwMiInterface;
    pMhwRender = m_renderHal->pMhwRenderInterface;
    iRemaining = 0;
    FlushParam = g_cRenderHal_InitMediaStateFlushParams;
    pPerfProfiler = m_renderHal->pPerfProfiler;
    pOsContext = pOsInterface->pOsContext;
    pMmioRegisters = pMhwRender->GetMmioRegisters();

    RENDER_PACKET_CHK_STATUS_RETURN(SetPowerMode(0));

    HalOcaInterface::On1stLevelBBStart(*commandBuffer, *pOsContext, pOsInterface->CurrentGpuContextHandle,
        *m_renderHal->pMhwMiInterface, *pMmioRegisters);
    OcaDumpDbgInfo(*commandBuffer, *pOsContext);

    RENDER_PACKET_CHK_STATUS_RETURN(SetMediaFrameTracking(GenericPrologParams));

    // Initialize command buffer and insert prolog
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnInitCommandBuffer(m_renderHal, commandBuffer, &GenericPrologParams));

    RENDER_PACKET_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectStartCmd((void*)m_renderHal, pOsInterface, pMhwMiInterface, commandBuffer));

    RENDER_PACKET_CHK_STATUS_RETURN(NullHW::StartPredicate(pMhwMiInterface, commandBuffer));

    // Write timing data for 3P budget
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendTimingData(m_renderHal, commandBuffer, true));

    bEnableSLM = false;  // Media walker first
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSetCacheOverrideParams(
        m_renderHal,
        &m_renderHal->L3CacheSettings,
        bEnableSLM));

    if (m_renderHal->bCmfcCoeffUpdate)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendCscCoeffSurface(m_renderHal,
            commandBuffer,
            m_renderHal->pCmfcCoeffSurface,
            m_renderHal->pStateHeap->pKernelAllocation[m_renderHal->iKernelAllocationID].pKernelEntry));
    }

    // Flush media states
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendMediaStates(
        m_renderHal,
        commandBuffer,
        (m_walkerType == WALKER_TYPE_MEDIA) ? &m_mediaWalkerParams : nullptr,
        &m_gpgpuWalkerParams));

    // Write back GPU Status tag
    if (!pOsInterface->bEnableKmdMediaFrameTracking)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendRcsStatusTag(m_renderHal, commandBuffer));
    }

    RENDER_PACKET_CHK_STATUS_RETURN(NullHW::StopPredicate(pMhwMiInterface, commandBuffer));

    RENDER_PACKET_CHK_STATUS_RETURN(pPerfProfiler->AddPerfCollectEndCmd((void*)m_renderHal, pOsInterface, pMhwMiInterface, commandBuffer));

    // Write timing data for 3P budget
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnSendTimingData(m_renderHal, commandBuffer, false));

    MHW_PIPE_CONTROL_PARAMS PipeControlParams;

    MOS_ZeroMemory(&PipeControlParams, sizeof(PipeControlParams));
    PipeControlParams.dwFlushMode = MHW_FLUSH_WRITE_CACHE;
    PipeControlParams.bGenericMediaStateClear = true;
    PipeControlParams.bIndirectStatePointersDisable = true;
    PipeControlParams.bDisableCSStall = false;

    RENDER_PACKET_CHK_NULL_RETURN(pOsInterface->pfnGetSkuTable);
    auto *skuTable = pOsInterface->pfnGetSkuTable(pOsInterface);
    if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
    {
        // Add PPC fulsh
        PipeControlParams.bPPCFlush = true;
    }
    RENDER_PACKET_CHK_STATUS_RETURN(AddPipeControl(pMhwMiInterface, commandBuffer));

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
        if (m_walkerType == WALKER_TYPE_MEDIA)
        {
            FlushParam.ui8InterfaceDescriptorOffset = m_mediaWalkerParams.InterfaceDescriptorOffset;
        }
        else
        {
            RENDER_PACKET_ASSERTMESSAGE("ERROR, pWalkerParams is nullptr and cannot get InterfaceDescriptorOffset.");
        }

        MediaStateFlush(pMhwMiInterface, commandBuffer, FlushParam);
    }
    else if (MEDIA_IS_WA(m_renderHal->pWaTable, WaAddMediaStateFlushCmd))
    {
        MediaStateFlush(pMhwMiInterface, commandBuffer, FlushParam);
    }

    HalOcaInterface::On1stLevelBBEnd(*commandBuffer, *pOsInterface);

    if (pBatchBuffer)
    {
        // Send Batch Buffer end command (HW/OS dependent)
        AddMiBatchBufferEnd(pMhwMiInterface, commandBuffer, nullptr);
    }
    else if (IsMiBBEndNeeded(pOsInterface))
    {
        // Send Batch Buffer end command for 1st level Batch Buffer
        AddMiBatchBufferEnd(pMhwMiInterface, commandBuffer, nullptr);
    }
    else if (m_renderHal->pOsInterface->bNoParsingAssistanceInKmd)
    {
        AddMiBatchBufferEnd(pMhwMiInterface, commandBuffer, nullptr);
    }

    // No need return command buffer here, which will be done in CmdTask::Submit.

    MOS_NULL_RENDERING_FLAGS  NullRenderingFlags;

    NullRenderingFlags =
        pOsInterface->pfnGetNullHWRenderFlags(pOsInterface);

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