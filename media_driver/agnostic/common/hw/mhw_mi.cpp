/*
* Copyright (c) 2014-2017, Intel Corporation
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
//! \file     mhw_mi.cpp
//! \brief    MHW interface for MI and generic flush commands across all engines
//! \details  Impelements the functionalities common across all platforms for MHW_MI
//!

#include "mhw_mi.h"
#include "mhw_cp_interface.h"

MhwMiInterface::MhwMiInterface(
    MhwCpInterface      *cpInterface,
    PMOS_INTERFACE      osInterface)
{
    MHW_FUNCTION_ENTER;

    MOS_ZeroMemory(&UseGlobalGtt, sizeof(UseGlobalGtt));
    MOS_ZeroMemory(&MediaResetParam, sizeof(MediaResetParam));

    if (cpInterface == nullptr || osInterface == nullptr)
    {
        MHW_ASSERTMESSAGE("Invalid input pointers provided");
        return;
    }

    if (!osInterface->bUsesGfxAddress && !osInterface->bUsesPatchList)
    {
        MHW_ASSERTMESSAGE("No valid addressing mode indicated");
        return;
    }

    if (cpInterface->RegisterMiInterface(this) != MOS_STATUS_SUCCESS)
    {
        MHW_ASSERTMESSAGE("Registering the MI interface should not have failed!");
        return;
    }

    m_cpInterface = cpInterface;
    m_osInterface = osInterface;

    UseGlobalGtt.m_cs   =
    UseGlobalGtt.m_vcs  =
    UseGlobalGtt.m_vecs = MEDIA_IS_WA(m_osInterface->pfnGetWaTable(m_osInterface), WaForceGlobalGTT) ||
                         !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrPPGTT);

    MediaResetParam.watchdogCountThreshold = MHW_MI_DEFAULT_WATCHDOG_THRESHOLD_IN_MS;

    GetWatchdogThreshold(m_osInterface);

    if (m_osInterface->bUsesGfxAddress)
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else // if (pOsInterface->bUsesPatchList)
    {
        AddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }
}

bool MhwMiInterface::IsGlobalGttInUse()
{
    MOS_GPU_CONTEXT gpuContext  = m_osInterface->pfnGetGpuContext(m_osInterface);
    bool vcsEngineUsed         = MOS_VCS_ENGINE_USED(gpuContext);
    bool renderEngineUsed      = MOS_RCS_ENGINE_USED(gpuContext);

    bool globalGttInUse        = renderEngineUsed ? UseGlobalGtt.m_cs :
        (vcsEngineUsed ? UseGlobalGtt.m_vcs : UseGlobalGtt.m_vecs);

    return globalGttInUse;
}

MOS_STATUS MhwMiInterface::AddProtectedProlog(MOS_COMMAND_BUFFER *cmdBuffer)
{
    MHW_MI_CHK_NULL(cmdBuffer);

    MHW_MI_CHK_STATUS(m_cpInterface->AddProlog(m_osInterface, cmdBuffer));
    MHW_MI_CHK_STATUS(m_cpInterface->AddCheckForEarlyExit(m_osInterface, cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

