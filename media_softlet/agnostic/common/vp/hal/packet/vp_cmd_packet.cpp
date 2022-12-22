/*
* Copyright (c) 2018, Intel Corporation
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
#include "vp_cmd_packet.h"
#include "vp_utils.h"

namespace vp {

VpCmdPacket::VpCmdPacket(
    MediaTask *task,
    PVP_MHWINTERFACE hwInterface,
    PVpAllocator &allocator,
    VPMediaMemComp *mmc,
    PacketType packetId)
    :CmdPacket(task),
    m_allocator(allocator),
    m_mmc(mmc),
    m_PacketId(packetId)
{
    m_surfSetting.Clean();
    m_hwInterface = hwInterface;
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_NO_STATUS_RETURN(m_hwInterface->m_osInterface);

    m_osInterface    = m_hwInterface->m_osInterface;
    m_userSettingPtr = m_hwInterface->m_osInterface->pfnGetUserSettingInstance(m_hwInterface->m_osInterface);
    m_report         = hwInterface->m_reporting;
}

MOS_STATUS VpCmdPacket::VpCmdPacketInit()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpCmdPacket::SetMediaFrameTracking(RENDERHAL_GENERIC_PROLOG_PARAMS &genericPrologParams)
{
    PMOS_INTERFACE  osInterface = nullptr;
    PMOS_RESOURCE   gpuStatusBuffer = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_hwInterface->m_osInterface);

    osInterface = m_hwInterface->m_osInterface;

#ifndef EMUL
    if(m_PacketCaps.lastSubmission && osInterface->bEnableKmdMediaFrameTracking)
    {
        // Get GPU Status buffer
        VP_PUBLIC_CHK_STATUS_RETURN(osInterface->pfnGetGpuStatusBufferResource(osInterface, gpuStatusBuffer));
        VP_PUBLIC_CHK_NULL_RETURN(gpuStatusBuffer);
        // Register the buffer
        VP_PUBLIC_CHK_STATUS_RETURN(osInterface->pfnRegisterResource(osInterface, gpuStatusBuffer, true, true));

        genericPrologParams.bEnableMediaFrameTracking = true;
        genericPrologParams.presMediaFrameTrackingSurface = gpuStatusBuffer;
        genericPrologParams.dwMediaFrameTrackingTag = osInterface->pfnGetGpuStatusTag(osInterface, osInterface->CurrentGpuContextOrdinal);
        genericPrologParams.dwMediaFrameTrackingAddrOffset = osInterface->pfnGetGpuStatusTagOffset(osInterface, osInterface->CurrentGpuContextOrdinal);

        VP_PUBLIC_CHK_NULL_RETURN(osInterface->pfnIncrementGpuStatusTag);
        // Increment GPU Status Tag
        osInterface->pfnIncrementGpuStatusTag(osInterface, osInterface->CurrentGpuContextOrdinal);
    }
#endif

    return MOS_STATUS_SUCCESS;
}

}