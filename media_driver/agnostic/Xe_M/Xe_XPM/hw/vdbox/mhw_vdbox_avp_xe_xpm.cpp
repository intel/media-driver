/*===================== begin_copyright_notice ==================================

# Copyright (c) 2020-2021, Intel Corporation

# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included
# in all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
# OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.

======================= end_copyright_notice ==================================*/
//!
//! \file     mhw_vdbox_avp_xe_xpm.cpp
//! \brief    Constructs VdBox AVP commands on Xe_XPM platforms

#include "mhw_vdbox_avp_xe_xpm.h"
#include "mhw_vdbox_avp_hwcmd_xe_xpm.h"
#include "mhw_utilities_xe_xpm.h"

MhwVdboxAvpInterfaceXe_Xpm::MhwVdboxAvpInterfaceXe_Xpm(
    PMOS_INTERFACE  osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface,
    bool            decodeInUse)
    : MhwVdboxAvpInterfaceG12(osInterface, miInterface, cpInterface, decodeInUse)
{
    MHW_FUNCTION_ENTER;
}

MhwVdboxAvpInterfaceXe_Xpm::~MhwVdboxAvpInterfaceXe_Xpm()
{
    MHW_FUNCTION_ENTER;
}

MOS_STATUS MhwVdboxAvpInterfaceXe_Xpm::AddAvpDecodeSurfaceStateCmd(
    PMOS_COMMAND_BUFFER              cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS        params)
    {
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);

    mhw_vdbox_avp_xe_xpm::AVP_SURFACE_STATE_CMD  *cmd =
        (mhw_vdbox_avp_xe_xpm::AVP_SURFACE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwVdboxAvpInterfaceG12::AddAvpDecodeSurfaceStateCmd(cmdBuffer, params));

    cmd->DW4.CompressionFormat = params->dwCompressionFormat;

    return eStatus;
}

MOS_STATUS MhwVdboxAvpInterfaceXe_Xpm::AddAvpPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    MhwVdboxAvpPipeBufAddrParams    *params)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->m_decodedPic);
    mhw_vdbox_avp_xe_xpm::AVP_PIPE_BUF_ADDR_STATE_CMD *cmd =
        (mhw_vdbox_avp_xe_xpm::AVP_PIPE_BUF_ADDR_STATE_CMD *)cmdBuffer->pCmdPtr;
    MHW_CHK_STATUS_RETURN(MhwVdboxAvpInterfaceG12::AddAvpPipeBufAddrCmd(cmdBuffer, params));

    cmd->DecodedOutputFrameBufferAddressAttributes.DW0.TileMode = MosGetHWTileType(params->m_decodedPic->TileType, params->m_decodedPic->TileModeGMM, params->m_decodedPic->bGMMTileEnabled);

    MOS_SURFACE     details;
    for (uint32_t i = 0; i < av1TotalRefsPerFrame; i++)
    {
        if (params->m_references[i] != nullptr)
        {
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, params->m_references[i], &details));
            cmd->ReferenceFrameBufferBaseAddressAttributes.DW0.TileMode = MosGetHWTileType(details.TileType, details.TileModeGMM, details.bGMMTileEnabled);
            break;
        }
    }

    //IntraBC Decoded Output Frame buffer
    if (params->m_intrabcDecodedOutputFrameBuffer != nullptr)
    {
        cmd->IntrabcDecodedOutputFrameBufferAddressAttributes.DW0.TileMode = MosGetHWTileType(params->m_decodedPic->TileType, params->m_decodedPic->TileModeGMM, params->m_decodedPic->bGMMTileEnabled);
    }

    return eStatus;
}
