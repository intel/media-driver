/*
* Copyright (c) 2020-2022, Intel Corporation
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
//! \file     media_debug_interface_misc.cpp
//! \brief    Defines the debug interface shared by all of Media.
//! \details  The debug interface dumps output from Media based on input config file.
//!
#include "media_debug_interface.h"
#if USE_MEDIA_DEBUG_TOOL
MOS_STATUS MediaDebugInterface::SubmitDummyWorkload(MOS_COMMAND_BUFFER *pCmdBuffer, int32_t bNullRendering)
{
    MHW_MI_FLUSH_DW_PARAMS flushDwParams{};
    MEDIA_DEBUG_CHK_STATUS(((MhwMiInterface*)m_miInterface)->AddMiFlushDwCmd(
        pCmdBuffer,
        &flushDwParams));
    MEDIA_DEBUG_CHK_STATUS(((MhwMiInterface*)m_miInterface)->AddMiBatchBufferEnd(
        pCmdBuffer,
        nullptr));
    m_osInterface->pfnReturnCommandBuffer(m_osInterface, pCmdBuffer, 0);
    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnSubmitCommandBuffer(m_osInterface, pCmdBuffer, bNullRendering));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MediaDebugInterface::CopySurfaceData_Vdbox(
    uint32_t      dwDataSize,
    PMOS_RESOURCE presSourceSurface,
    PMOS_RESOURCE presCopiedSurface)
{
    MOS_COMMAND_BUFFER        CmdBuffer{};
    MHW_MI_FLUSH_DW_PARAMS    FlushDwParams{};
    MHW_GENERIC_PROLOG_PARAMS genericPrologParams{};
    MHW_CP_COPY_PARAMS        cpCopyParams{};
    MOS_NULL_RENDERING_FLAGS  NullRenderingFlags{};
    MOS_GPU_CONTEXT           orgGpuContext{};

    if (!m_vdboxContextCreated)
    {
        MOS_GPUCTX_CREATOPTIONS_ENHANCED createOption = {};

        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnCreateGpuContext(
            m_osInterface,
            MOS_GPU_CONTEXT_VIDEO,
            MOS_GPU_NODE_VIDEO,
            &createOption));

        // Register VDbox GPU context with the Batch Buffer completion event
        MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
            m_osInterface,
            MOS_GPU_CONTEXT_VIDEO));

        m_vdboxContextCreated = true;
    }

    MEDIA_DEBUG_CHK_NULL(m_cpInterface);
    MEDIA_DEBUG_CHK_NULL(m_osInterface);
    MEDIA_DEBUG_CHK_NULL(m_miInterface);
    MEDIA_DEBUG_CHK_NULL(m_osInterface->pfnGetWaTable(m_osInterface));

    orgGpuContext = m_osInterface->CurrentGpuContextOrdinal;

    // Due to VDBOX cryto copy limitation, the size must be Cache line aligned
    if (!MOS_IS_ALIGNED(dwDataSize, MHW_CACHELINE_SIZE))
    {
        MEDIA_DEBUG_ASSERTMESSAGE("Size is not CACHE line aligned, cannot use VDBOX to copy.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnSetGpuContext(m_osInterface, MOS_GPU_CONTEXT_VIDEO));
    m_osInterface->pfnResetOsStates(m_osInterface);

    // Register the target resource
    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnRegisterResource(
        m_osInterface,
        presCopiedSurface,
        true,
        true));

    // Register the source resource
    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnRegisterResource(
        m_osInterface,
        presSourceSurface,
        false,
        true));

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnGetCommandBuffer(m_osInterface, &CmdBuffer, 0));

    genericPrologParams.pOsInterface  = m_osInterface;
    genericPrologParams.pvMiInterface = (MhwMiInterface*)m_miInterface;
    genericPrologParams.bMmcEnabled   = false;
    MEDIA_DEBUG_CHK_STATUS(Mhw_SendGenericPrologCmd(&CmdBuffer, &genericPrologParams));

    cpCopyParams.size          = dwDataSize;
    cpCopyParams.presSrc       = presSourceSurface;
    cpCopyParams.presDst       = presCopiedSurface;
    cpCopyParams.isEncodeInUse = false;

    MEDIA_DEBUG_CHK_STATUS(m_cpInterface->SetCpCopy(m_osInterface, &CmdBuffer, &cpCopyParams));

    // MI_FLUSH
    MEDIA_DEBUG_CHK_STATUS(((MhwMiInterface*)m_miInterface)->AddMiFlushDwCmd(
        &CmdBuffer,
        &FlushDwParams));

    MEDIA_DEBUG_CHK_STATUS(((MhwMiInterface*)m_miInterface)->AddMiBatchBufferEnd(
        &CmdBuffer,
        nullptr));

    m_osInterface->pfnReturnCommandBuffer(m_osInterface, &CmdBuffer, 0);

    NullRenderingFlags = m_osInterface->pfnGetNullHWRenderFlags(m_osInterface);

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnSubmitCommandBuffer(
        m_osInterface,
        &CmdBuffer,
        NullRenderingFlags.CtxVideo || NullRenderingFlags.CodecGlobal || NullRenderingFlags.CtxVideo || NullRenderingFlags.VPGobal));

    MEDIA_DEBUG_CHK_STATUS(m_osInterface->pfnSetGpuContext(m_osInterface, orgGpuContext));
    return MOS_STATUS_SUCCESS;
}

#endif  // USE_MEDIA_DEBUG_TOOL
