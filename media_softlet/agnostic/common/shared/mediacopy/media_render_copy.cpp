/*
* Copyright (c) 2020, Intel Corporation
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
//! \file       media_render_copy.cpp
//! \brief      render copy implement file
//! \details    render copy implement file
//!

#include "media_render_copy.h"

RenderCopyState::RenderCopyState(PMOS_INTERFACE osInterface, MhwInterfaces *mhwInterfaces) :
    m_osInterface(osInterface),
    m_mhwInterfaces(mhwInterfaces)
{
    m_renderInterface = mhwInterfaces->m_renderInterface;
}

RenderCopyState:: ~RenderCopyState()
{
    if (m_renderHal != nullptr)
    {
       MOS_STATUS eStatus = m_renderHal->pfnDestroy(m_renderHal);
       if (eStatus != MOS_STATUS_SUCCESS)
       {
           MCPY_ASSERTMESSAGE("Failed to destroy RenderHal, eStatus:%d.\n", eStatus);
       }
       MOS_FreeMemAndSetNull(m_renderHal);
    }

    if (m_cpInterface != nullptr)
    {
        Delete_MhwCpInterface(m_cpInterface);
        m_cpInterface = nullptr;
    }

}

MOS_STATUS RenderCopyState::Initialize()
{
    MOS_GPU_NODE            RenderGpuNode;
    MOS_GPU_CONTEXT         RenderGpuContext;
    MOS_GPUCTX_CREATOPTIONS createOption;
    RENDERHAL_SETTINGS      RenderHalSettings;

    RenderGpuContext   = MOS_GPU_CONTEXT_COMPUTE;
    RenderGpuNode      = MOS_GPU_NODE_COMPUTE;

    MCPY_CHK_NULL_RETURN(m_osInterface);

    Mos_SetVirtualEngineSupported(m_osInterface, true);
    Mos_CheckVirtualEngineSupported(m_osInterface, true, true);
    // Create render copy Context
    MCPY_CHK_STATUS_RETURN(m_osInterface->pfnCreateGpuContext(
        m_osInterface,
        RenderGpuContext,
        RenderGpuNode,
        &createOption));

    // Register context with the Batch Buffer completion event
    MCPY_CHK_STATUS_RETURN(m_osInterface->pfnRegisterBBCompleteNotifyEvent(
        m_osInterface,
        RenderGpuContext));

    m_renderHal = (PRENDERHAL_INTERFACE)MOS_AllocAndZeroMemory(sizeof(*m_renderHal));
    MCPY_CHK_NULL_RETURN(m_renderHal);
    MCPY_CHK_STATUS_RETURN(RenderHal_InitInterface(
        m_renderHal,
        &m_cpInterface,
        m_osInterface));

    // Allocate and initialize HW states
    RenderHalSettings.iMediaStates = 32;
    MCPY_CHK_STATUS_RETURN(m_renderHal->pfnInitialize(m_renderHal, &RenderHalSettings));

    m_renderHal->sseuTable = VpDefaultSSEUTable;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS RenderCopyState::SubmitCMD( )
{
    return MOS_STATUS_SUCCESS;
}