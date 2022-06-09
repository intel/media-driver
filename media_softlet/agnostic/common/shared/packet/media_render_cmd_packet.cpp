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
}

MOS_STATUS RenderCmdPacket::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
{
    PMOS_INTERFACE                      pOsInterface = nullptr;
    MOS_STATUS                          eStatus = MOS_STATUS_SUCCESS;
    uint32_t                            dwSyncTag = 0;
    int32_t                             i = 0, iRemaining = 0;
    MHW_MEDIA_STATE_FLUSH_PARAM         FlushParam = {};
    bool                                bEnableSLM = false;
    RENDERHAL_GENERIC_PROLOG_PARAMS     GenericPrologParams = {};
    MOS_RESOURCE                        GpuStatusBuffer = {};
    MediaPerfProfiler*                  pPerfProfiler = nullptr;
    MOS_CONTEXT*                        pOsContext = nullptr;
    PMHW_MI_MMIOREGISTERS               pMmioRegisters = nullptr;

    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pRenderHalPltInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pRenderHalPltInterface->GetMmioRegisters(m_renderHal));
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pOsInterface);
    RENDER_PACKET_CHK_NULL_RETURN(m_renderHal->pOsInterface->pOsContext);
    RENDER_PACKET_CHK_NULL_RETURN(commandBuffer);

    eStatus = MOS_STATUS_UNKNOWN;
    pOsInterface = m_renderHal->pOsInterface;
    iRemaining = 0;
    FlushParam = g_cRenderHal_InitMediaStateFlushParams;
    pPerfProfiler = m_renderHal->pPerfProfiler;
    pOsContext = pOsInterface->pOsContext;
    pMmioRegisters = m_renderHal->pRenderHalPltInterface->GetMmioRegisters(m_renderHal);

    RENDER_PACKET_CHK_STATUS_RETURN(SetPowerMode(0));

    m_renderHal->pRenderHalPltInterface->On1stLevelBBStart(m_renderHal, commandBuffer, pOsContext, pOsInterface->CurrentGpuContextHandle, pMmioRegisters);

    OcaDumpDbgInfo(*commandBuffer, *pOsContext);

    RENDER_PACKET_CHK_STATUS_RETURN(SetMediaFrameTracking(GenericPrologParams));

    // Initialize command buffer and insert prolog
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pfnInitCommandBuffer(m_renderHal, commandBuffer, &GenericPrologParams));

    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddPerfCollectStartCmd(m_renderHal, pOsInterface, commandBuffer));

    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->StartPredicate(m_renderHal, commandBuffer));

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

    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->StopPredicate(m_renderHal, commandBuffer));

    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddPerfCollectEndCmd(m_renderHal, pOsInterface, commandBuffer));

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
    RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddMiPipeControl(m_renderHal, commandBuffer, &PipeControlParams));

    if (MEDIA_IS_WA(m_renderHal->pWaTable, WaSendDummyVFEafterPipelineSelect))
    {
        MHW_VFE_PARAMS VfeStateParams = {};
        VfeStateParams.dwNumberofURBEntries = 1;
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddMediaVfeCmd(m_renderHal, commandBuffer, &VfeStateParams));
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

        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddMediaStateFlush(m_renderHal, commandBuffer, &FlushParam));
    }
    else if (MEDIA_IS_WA(m_renderHal->pWaTable, WaAddMediaStateFlushCmd))
    {
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddMediaStateFlush(m_renderHal, commandBuffer, &FlushParam));
    }

    HalOcaInterface::On1stLevelBBEnd(*commandBuffer, *pOsInterface);

    if (pBatchBuffer)
    {
        // Send Batch Buffer end command (HW/OS dependent)
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddMiBatchBufferEnd(m_renderHal, commandBuffer, nullptr));
    }
    else if (IsMiBBEndNeeded(pOsInterface))
    {
        // Send Batch Buffer end command for 1st level Batch Buffer
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddMiBatchBufferEnd(m_renderHal, commandBuffer, nullptr));
    }
    else if (m_renderHal->pOsInterface->bNoParsingAssistanceInKmd)
    {
        RENDER_PACKET_CHK_STATUS_RETURN(m_renderHal->pRenderHalPltInterface->AddMiBatchBufferEnd(m_renderHal, commandBuffer, nullptr));
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
