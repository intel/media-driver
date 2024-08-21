/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     decode_mem_compression_xe2_hpm.cpp
//! \brief    Defines the common interface for decode mmc.
//! \details  The decode mmc is to handle mmc operations
//!

#include "decode_mem_compression_xe2_hpm.h"
#include "decode_utils.h"

DecodeMemCompXe2_Hpm::DecodeMemCompXe2_Hpm(
    CodechalHwInterfaceNext *hwInterface):
    DecodeMemComp(hwInterface)
{

}

MOS_STATUS DecodeMemCompXe2_Hpm::SendPrologCmd(PMOS_COMMAND_BUFFER cmdBuffer, bool bRcsIsUsed)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMemCompXe2_Hpm::SetSurfaceMmcState(PMOS_SURFACE surface)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMemCompXe2_Hpm::SetSurfaceMmcMode(PMOS_SURFACE surface)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMemCompXe2_Hpm::GetSurfaceMmcState(PMOS_SURFACE surface, MOS_MEMCOMP_STATE *mmcMode)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeMemCompXe2_Hpm::GetResourceMmcState(PMOS_RESOURCE resource, MOS_MEMCOMP_STATE &mmcMode)
{
    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS DecodeMemCompXe2_Hpm::UpdateUserFeatureKey(PMOS_SURFACE surface)
{
    if (!surface)
        return MOS_STATUS_NULL_POINTER;

    if (m_userFeatureUpdated)
    {
        return MOS_STATUS_SUCCESS;
    }
    m_userFeatureUpdated = true;

    ReportUserSetting(
        m_userSettingPtr,
        "Decode RT Compressible",
        surface->bCompressible,
        MediaUserSetting::Group::Sequence);

    MOS_MEMCOMP_STATE surfaceMmcMode = MOS_MEMCOMP_DISABLED;
    if (m_mmcEnabled)
    {
        m_osInterface->pfnGetMemoryCompressionMode(m_osInterface, &surface->OsResource, &surfaceMmcMode);
    }
    else
    {
        surfaceMmcMode = MOS_MEMCOMP_DISABLED;
    }

    ReportUserSetting(
        m_userSettingPtr,
        "Decode RT Compress Mode",
        surfaceMmcMode,
        MediaUserSetting::Group::Sequence);

    return MOS_STATUS_SUCCESS;
}
#endif
