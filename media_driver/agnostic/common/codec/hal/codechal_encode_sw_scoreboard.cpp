/*
* Copyright (c) 2017-2018, Intel Corporation
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
//! \file     codechal_encode_sw_scoreboard.cpp
//! \brief    Defines base class for SW scoreboard init kernel
//!

#include "codechal_encoder_base.h"
#include "codechal_encode_sw_scoreboard.h"
#include "codeckrnheader.h"
#include "hal_oca_interface.h"

MOS_STATUS CodechalEncodeSwScoreboard::AllocateResources()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if (Mos_ResourceIsNull(&m_surfaceParams.swScoreboardSurface[m_surfaceParams.surfaceIndex].OsResource))
    {
        MOS_ZeroMemory(&m_surfaceParams.swScoreboardSurface[m_surfaceParams.surfaceIndex], sizeof(MOS_SURFACE));

        MOS_ALLOC_GFXRES_PARAMS  allocParamsForBuffer2D;
        MOS_ZeroMemory(&allocParamsForBuffer2D, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBuffer2D.Type     = MOS_GFXRES_2D;
        allocParamsForBuffer2D.TileType = MOS_TILE_LINEAR;
        allocParamsForBuffer2D.Format   = Format_R32U;
        allocParamsForBuffer2D.dwWidth  = m_surfaceParams.swScoreboardSurfaceWidth;
        allocParamsForBuffer2D.dwHeight = m_surfaceParams.swScoreboardSurfaceHeight;
        allocParamsForBuffer2D.pBufName = "SW scoreboard init Buffer";
        CODECHAL_ENCODE_CHK_STATUS_MESSAGE_RETURN(m_osInterface->pfnAllocateResource(
            m_osInterface,
            &allocParamsForBuffer2D,
            &m_surfaceParams.swScoreboardSurface[m_surfaceParams.surfaceIndex].OsResource),
            "Failed to allocate SW scoreboard init Buffer.");

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalGetResourceInfo(m_osInterface,
            &m_surfaceParams.swScoreboardSurface[m_surfaceParams.surfaceIndex]));
    }

    return eStatus;
}

void CodechalEncodeSwScoreboard::ReleaseResources()
{
    for (auto i = 0; i < CODECHAL_ENCODE_SW_SCOREBOARD_SURFACE_NUM; i++)
    {
        if (!Mos_ResourceIsNull(&m_surfaceParams.swScoreboardSurface[i].OsResource))
        {
            m_osInterface->pfnFreeResource(
                m_osInterface,
                &m_surfaceParams.swScoreboardSurface[i].OsResource);
        }
    }
}

uint8_t CodechalEncodeSwScoreboard::GetBTCount()
{
    return (uint8_t)swScoreboardNumSurfaces;
}

MOS_STATUS CodechalEncodeSwScoreboard::SetCurbe()
{
    CODECHAL_ENCODE_FUNCTION_ENTER;

    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    CurbeData                   curbe;

    MOS_ZeroMemory(&curbe, sizeof(CurbeData));

    // ScoreboardWidth and Height in units of Thread Space
    curbe.DW0.scoreboardWidth          = m_curbeParams.scoreboardWidth;
    curbe.DW0.scoreboardHeight         = m_curbeParams.scoreboardHeight;
    curbe.DW1.isHevc                   = m_curbeParams.isHevc;
    curbe.DW2.numberOfWaveFrontsSplits = m_curbeParams.numberOfWaveFrontSplit;
    curbe.DW2.numberofChildThreads     = m_curbeParams.numberOfChildThread;
    curbe.DW4.dependencyPattern        = m_dependencyPatternIdx;
    curbe.DW16.softwareScoreboard      = swScoreboardInitSurface;
    curbe.DW17.lcuInfoSurface          = swScoreboardInitLcuInfoSurface;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_kernelState->m_dshRegion.AddData(
        &curbe,
        m_kernelState->dwCurbeOffset,
        sizeof(curbe)));

    return eStatus;
}

MOS_STATUS CodechalEncodeSwScoreboard::SendSurface(PMOS_COMMAND_BUFFER cmdBuffer)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(cmdBuffer);

    // Set Surface States
    CODECHAL_SURFACE_CODEC_PARAMS surfaceCodecParams;
    MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
    surfaceCodecParams.bIs2DSurface            = true;
    surfaceCodecParams.bMediaBlockRW           = true;
    surfaceCodecParams.bIsWritable             = true;
    surfaceCodecParams.bRenderTarget           = true;
    surfaceCodecParams.psSurface               = &m_surfaceParams.swScoreboardSurface[m_surfaceParams.surfaceIndex];
    surfaceCodecParams.dwCacheabilityControl   = m_hwInterface->GetCacheabilitySettings()[MOS_CODEC_RESOURCE_USAGE_SURFACE_SOFTWARE_SCOREBOARD_ENCODE].Value;
    surfaceCodecParams.dwBindingTableOffset    = swScoreboardInitSurface;
    surfaceCodecParams.bUse32UINTSurfaceFormat = true;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
        m_hwInterface,
        cmdBuffer,
        &surfaceCodecParams,
        m_kernelState));

    if (m_surfaceParams.lcuInfoSurface)
    {
        MOS_ZeroMemory(&surfaceCodecParams, sizeof(surfaceCodecParams));
        surfaceCodecParams.bIs2DSurface            = true;
        surfaceCodecParams.bMediaBlockRW           = true;
        surfaceCodecParams.bIsWritable             = true;
        surfaceCodecParams.bRenderTarget           = true;
        surfaceCodecParams.psSurface               = m_surfaceParams.lcuInfoSurface;
        surfaceCodecParams.dwBindingTableOffset    = swScoreboardInitLcuInfoSurface;
        surfaceCodecParams.bUse32UINTSurfaceFormat = true;

        CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalSetRcsSurfaceState(
            m_hwInterface,
            cmdBuffer,
            &surfaceCodecParams,
            m_kernelState));
    }

    return eStatus;
}

MOS_STATUS CodechalEncodeSwScoreboard::Execute(KernelParams *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    CODECHAL_ENCODE_FUNCTION_ENTER;

    CODECHAL_ENCODE_CHK_NULL_RETURN(params);

    PerfTagSetting perfTag;
    CODECHAL_ENCODE_SET_PERFTAG_INFO(perfTag, CODECHAL_ENCODE_PERFTAG_CALL_SCOREBOARD);

    // Allocate output surface
    m_surfaceParams.isHevc                    = params->isHevc;
    m_surfaceParams.swScoreboardSurfaceWidth  = params->swScoreboardSurfaceWidth;
    m_surfaceParams.swScoreboardSurfaceHeight = params->swScoreboardSurfaceHeight;
    if (m_surfaceParams.isHevc)
    {
        m_surfaceParams.lcuInfoSurface         = params->lcuInfoSurface;
    }

    CODECHAL_ENCODE_CHK_STATUS_RETURN(AllocateResources());

    // If Single Task Phase is not enabled, use BT count for the kernel state.
    if (m_firstTaskInPhase == true || !m_singleTaskPhaseSupported)
    {
        auto maxBtCount = m_singleTaskPhaseSupported ?
            m_maxBtCount : m_kernelState->KernelParams.iBTCount;
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->RequestSshSpaceForCmdBuf(maxBtCount));
        m_vmeStatesSize = m_hwInterface->GetKernelLoadCommandSize(maxBtCount);
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->VerifySpaceAvailable());
    }

    // setup DSH and Interface Descriptor
    auto stateHeapInterface = m_renderInterface->m_stateHeapInterface;
    CODECHAL_ENCODE_CHK_NULL_RETURN(stateHeapInterface);
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->AssignDshAndSshSpace(
        stateHeapInterface,
        m_kernelState,
        false,
        0,
        false,
        m_storeData));

    MHW_INTERFACE_DESCRIPTOR_PARAMS idParams;
    MOS_ZeroMemory(&idParams, sizeof(idParams));
    idParams.pKernelState = m_kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SetInterfaceDescriptor(1, &idParams));

    // Setup Curbe
    m_curbeParams.scoreboardWidth        = params->scoreboardWidth;
    m_curbeParams.scoreboardHeight       = params->scoreboardHeight;
    m_curbeParams.isHevc                 = params->isHevc;
    m_curbeParams.numberOfWaveFrontSplit = params->numberOfWaveFrontSplit;
    m_curbeParams.numberOfChildThread    = params->numberOfChildThread;

    CODECHAL_ENCODE_CHK_STATUS_RETURN(SetCurbe());

    CODECHAL_MEDIA_STATE_TYPE encFunctionType = CODECHAL_MEDIA_STATE_SW_SCOREBOARD_INIT;
    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_DSH_TYPE,
            m_kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCurbe(
            encFunctionType,
            m_kernelState));
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_ISH_TYPE,
            m_kernelState));
    )

    MOS_COMMAND_BUFFER cmdBuffer;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_osInterface->pfnGetCommandBuffer(m_osInterface, &cmdBuffer, 0));

    SendKernelCmdsParams sendKernelCmdsParams = SendKernelCmdsParams();
    sendKernelCmdsParams.EncFunctionType = encFunctionType;
    sendKernelCmdsParams.pKernelState = m_kernelState;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->SendGenericKernelCmds(&cmdBuffer, &sendKernelCmdsParams));

    // add binding table
    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SetBindingTable(m_kernelState));

    // send surfaces
    CODECHAL_ENCODE_CHK_STATUS_RETURN(SendSurface(&cmdBuffer));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            m_kernelState));
    )

    // Thread Dispatch Pattern - MEDIA OBJECT WALKER
    CODECHAL_WALKER_CODEC_PARAMS walkerCodecParams;
    MOS_ZeroMemory(&walkerCodecParams, sizeof(walkerCodecParams));
    walkerCodecParams.WalkerMode              = m_walkerMode;
    walkerCodecParams.dwResolutionX           = (uint32_t)(ceil)((m_curbeParams.scoreboardWidth) / 4.0);
    walkerCodecParams.dwResolutionY           = (uint32_t)(ceil)((m_curbeParams.scoreboardHeight) / 4.0);
    walkerCodecParams.bNoDependency           = true;
    walkerCodecParams.bGroupIdSelectSupported = m_groupIdSelectSupported;

    MHW_WALKER_PARAMS walkerParams;
    CODECHAL_ENCODE_CHK_STATUS_RETURN(CodecHalInitMediaObjectWalkerParams(m_hwInterface, &walkerParams, &walkerCodecParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_renderInterface->AddMediaObjectWalkerCmd(&cmdBuffer, &walkerParams));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_encoder->EndStatusReport(&cmdBuffer, encFunctionType));

    CODECHAL_DEBUG_TOOL(
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpKernelRegion(
            encFunctionType,
            MHW_SSH_TYPE,
            m_kernelState));

        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr));
    )

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->SubmitBlocks(m_kernelState));
    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_stateHeapInterface->UpdateGlobalCmdBufId());
        CODECHAL_ENCODE_CHK_STATUS_RETURN(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    CODECHAL_DEBUG_TOOL(CODECHAL_ENCODE_CHK_STATUS_RETURN(m_debugInterface->DumpCmdBuffer(
            &cmdBuffer,
            encFunctionType,
            nullptr)));

    CODECHAL_ENCODE_CHK_STATUS_RETURN(m_hwInterface->UpdateSSEuForCmdBuffer(
        &cmdBuffer, m_singleTaskPhaseSupported, m_lastTaskInPhase));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &cmdBuffer, 0);

    if (!m_singleTaskPhaseSupported || m_lastTaskInPhase)
    {
        HalOcaInterface::On1stLevelBBEnd(cmdBuffer, *m_osInterface->pOsContext);
        m_osInterface->pfnSubmitCommandBuffer(m_osInterface, &cmdBuffer, m_renderContextUsesNullHw);
        m_lastTaskInPhase = false;
    }

    return eStatus;
}

CodechalEncodeSwScoreboard::CodechalEncodeSwScoreboard(CodechalEncoderState *encoder)
    : m_useHwScoreboard(encoder->m_useHwScoreboard),
    m_renderContextUsesNullHw(encoder->m_renderContextUsesNullHw),
    m_groupIdSelectSupported(encoder->m_groupIdSelectSupported),
    m_singleTaskPhaseSupported(encoder->m_singleTaskPhaseSupported),
    m_firstTaskInPhase(encoder->m_firstTaskInPhase),
    m_lastTaskInPhase(encoder->m_lastTaskInPhase),
    m_groupId(encoder->m_groupId),
    m_pictureCodingType(encoder->m_pictureCodingType),
    m_mode(encoder->m_mode),
    m_verticalLineStride(encoder->m_verticalLineStride),
    m_maxBtCount(encoder->m_maxBtCount),
    m_vmeStatesSize(encoder->m_vmeStatesSize),
    m_storeData(encoder->m_storeData),
    m_walkerMode(encoder->m_walkerMode)
{
    CODECHAL_ENCODE_CHK_NULL_NO_STATUS_RETURN(encoder);

    // Initilize interface pointers
    m_encoder               = encoder;
    m_osInterface           = encoder->GetOsInterface();
    m_hwInterface           = encoder->GetHwInterface();
    m_debugInterface        = encoder->GetDebugInterface();
    m_miInterface           = m_hwInterface->GetMiInterface();
    m_renderInterface       = m_hwInterface->GetRenderInterface();
    m_stateHeapInterface    = m_renderInterface->m_stateHeapInterface->pStateHeapInterface;
    m_curbeLength           = sizeof(CurbeData);
    m_kernelBase            = encoder->m_kernelBase;
    m_kuidCommon            = IDR_CODEC_HME_DS_SCOREBOARD_KERNEL;
}

CodechalEncodeSwScoreboard::~CodechalEncodeSwScoreboard()
{
    // free SW scoreboard init surface
    ReleaseResources();

    if (m_kernelState)
    {
        MOS_Delete(m_kernelState);
        m_kernelState = nullptr;
    }
}