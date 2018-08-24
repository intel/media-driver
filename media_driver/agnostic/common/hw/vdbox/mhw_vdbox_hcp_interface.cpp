/*
* Copyright (c) 2014-2018, Intel Corporation
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

//! \file     mhw_vdbox_hcp_interface.cpp
//! \brief    MHW interface for constructing HCP commands for the Vdbox engine
//! \details  Defines the interfaces for constructing MHW Vdbox HCP commands across all platforms 
//!

#include "mhw_vdbox_hcp_interface.h"

const MhwVdboxHcpInterface::HevcSliceType MhwVdboxHcpInterface::m_hevcBsdSliceType[3] =
{
    MhwVdboxHcpInterface::hevcSliceB,
    MhwVdboxHcpInterface::hevcSliceP,
    MhwVdboxHcpInterface::hevcSliceI
};

MOS_STATUS MhwVdboxHcpInterface::AddHcpSurfaceCmd(
    PMOS_COMMAND_BUFFER                  cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS            params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddHcpDecodeSurfaceStateCmd(cmdBuffer, params));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddHcpEncodeSurfaceStateCmd(cmdBuffer, params));
    }

    return eStatus;
}

MhwVdboxHcpInterface::MhwVdboxHcpInterface(
    PMOS_INTERFACE osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface,
    bool decodeInUse)
{
    MHW_FUNCTION_ENTER;

    m_osInterface = osInterface;
    m_miInterface = miInterface;
    m_cpInterface = cpInterface;
    m_decodeInUse = decodeInUse;

    MHW_ASSERT(m_osInterface);
    MHW_ASSERT(m_miInterface);
    MHW_ASSERT(m_cpInterface);

    m_waTable = osInterface->pfnGetWaTable(osInterface);
    m_skuTable = osInterface->pfnGetSkuTable(osInterface);

    if (m_osInterface->bUsesGfxAddress)
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_GfxAddress;
    }
    else // bUsesPatchList
    {
        pfnAddResourceToCmd = Mhw_AddResourceToCmd_PatchList;
    }
 }

MOS_STATUS MhwVdboxHcpInterface::AddHcpPicStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_PIC_STATE        params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddHcpDecodePicStateCmd(cmdBuffer, params));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddHcpEncodePicStateCmd(cmdBuffer, params));
    }

    return eStatus;
}

MOS_STATUS MhwVdboxHcpInterface::AddHcpSliceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_HEVC_SLICE_STATE      hevcSliceState)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_FUNCTION_ENTER;

    if (m_decodeInUse)
    {
        MHW_MI_CHK_STATUS(AddHcpDecodeSliceStateCmd(cmdBuffer, hevcSliceState));
    }
    else
    {
        MHW_MI_CHK_STATUS(AddHcpEncodeSliceStateCmd(cmdBuffer, hevcSliceState));
    }

    return eStatus;
}

uint16_t MhwVdboxHcpInterface::Convert2SignMagnitude(
    int32_t val,
    uint32_t signBitPos)
{
    uint16_t retVal = 0;
    if (val < 0)
    {
        retVal = ((1 << (signBitPos - 1)) | (-val & ((1 << (signBitPos - 1)) - 1)));
    }
    else
    {
        retVal = val & ((1 << (signBitPos - 1)) - 1);
    }
    return retVal;
}
