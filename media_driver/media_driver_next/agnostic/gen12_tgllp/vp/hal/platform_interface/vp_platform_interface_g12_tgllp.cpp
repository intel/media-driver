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
//! \file     vp_platform_interface_g12_tgllp.cpp
//! \brief    Factories for vp object creation.
//!
#include "vp_feature_manager_m12_0.h"
#include "vp_platform_interface_g12_tgllp.h"
#include "vp_vebox_cmd_packet_g12.h"

using namespace vp;

MOS_STATUS VpPlatformInterfaceG12Tgllp::InitVpVeboxSfcHwCaps(VP_VEBOX_ENTRY_REC *veboxHwEntry, uint32_t veboxEntryCount,
                                                            VP_SFC_ENTRY_REC *sfcHwEntry, uint32_t sfcEntryCount)
{
    if (veboxEntryCount < Format_Count || sfcEntryCount < Format_Count ||
        nullptr == veboxHwEntry || nullptr == sfcHwEntry)
    {
        veboxEntryCount = Format_Count;
        sfcEntryCount = Format_Count;
        return MOS_STATUS_INVALID_PARAMETER;
    }
#include "vp_feature_caps_g12.h"
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPlatformInterfaceG12Tgllp::InitVpRenderHwCaps()
{
    return MOS_STATUS_SUCCESS;
}

VPFeatureManager *VpPlatformInterfaceG12Tgllp::CreateFeatureChecker(_VP_MHWINTERFACE *hwInterface)
{
    return MOS_New(VPFeatureManagerM12_0, hwInterface);
}

VpCmdPacket *VpPlatformInterfaceG12Tgllp::CreateVeboxPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
{
    return MOS_New(VpVeboxCmdPacketG12, task, hwInterface, allocator, mmc);
}

VpCmdPacket *VpPlatformInterfaceG12Tgllp::CreateRenderPacket(MediaTask * task, _VP_MHWINTERFACE *hwInterface, VpAllocator *&allocator, VPMediaMemComp *mmc)
{
    return nullptr;
}