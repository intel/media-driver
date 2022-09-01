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
//! \file     mhw_vdbox_mfx_xe_xpm.cpp
//! \brief    Constructs VdBox MFX commands on Gen12-based platforms

#include "mhw_vdbox_mfx_xe_xpm.h"
#include "mhw_vdbox_mfx_hwcmd_xe_xpm.h"
#include "mhw_utilities_xe_xpm.h"

MhwVdboxMfxInterfaceXe_Xpm::MhwVdboxMfxInterfaceXe_Xpm(
    PMOS_INTERFACE  osInterface,
    MhwMiInterface *miInterface,
    MhwCpInterface *cpInterface,
    bool            decodeInUse)
    : MhwVdboxMfxInterfaceG12(osInterface, miInterface, cpInterface, decodeInUse)
{
    MHW_FUNCTION_ENTER;
}
MhwVdboxMfxInterfaceXe_Xpm ::~MhwVdboxMfxInterfaceXe_Xpm()
{
    MHW_FUNCTION_ENTER;
}

MOS_STATUS MhwVdboxMfxInterfaceXe_Xpm::AddMfxSurfaceCmd(
    PMOS_COMMAND_BUFFER       cmdBuffer,
    PMHW_VDBOX_SURFACE_PARAMS params)
{
    MOS_STATUS                                    eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->psSurface);

    mhw_vdbox_mfx_xe_xpm::MFX_SURFACE_STATE_CMD *cmd =
        (mhw_vdbox_mfx_xe_xpm::MFX_SURFACE_STATE_CMD *)cmdBuffer->pCmdPtr;

    MHW_CHK_STATUS_RETURN(MhwVdboxMfxInterfaceG12::AddMfxSurfaceCmd(cmdBuffer, params));
    cmd->DW3.TileMode          = MosGetHWTileType(params->psSurface->TileType, params->psSurface->TileModeGMM, params->psSurface->bGMMTileEnabled);
    cmd->DW3.CompressionFormat = params->dwCompressionFormat;

    return eStatus;
}

MOS_STATUS MhwVdboxMfxInterfaceXe_Xpm::AddMfxPipeModeSelectCmd(
    PMOS_COMMAND_BUFFER                cmdBuffer,
    PMHW_VDBOX_PIPE_MODE_SELECT_PARAMS params)
{
    auto *cmd = (mhw_vdbox_mfx_xe_xpm::MFX_PIPE_MODE_SELECT_CMD *)(cmdBuffer->pCmdPtr
        + mhw_mi_g12_X::MFX_WAIT_CMD::dwSize);

    MHW_CHK_STATUS_RETURN(MhwVdboxMfxInterfaceG12::AddMfxPipeModeSelectCmd(cmdBuffer, params));
    cmd->DW1.ScaledSurfaceEnable = 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxMfxInterfaceXe_Xpm::AddMfxPipeBufAddrCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VDBOX_PIPE_BUF_ADDR_PARAMS params)
{
    auto *cmd           = (mhw_vdbox_mfx_xe_xpm::MFX_PIPE_BUF_ADDR_STATE_CMD *)cmdBuffer->pCmdPtr;
    auto  ps4xDsSurface = params->ps4xDsSurface;

    params->ps4xDsSurface = nullptr;  // XeHP does not add this surface

    MHW_CHK_STATUS_RETURN(MhwVdboxMfxInterfaceG12::AddMfxPipeBufAddrCmd(cmdBuffer, params));
    cmd->DW3.Reserved108 = 0;

    // params is also used for VDENC and this surface is needed for VDENC pipe buffer address
    params->ps4xDsSurface = ps4xDsSurface;

    // Override mmc state for per frame control
    PMOS_RESOURCE *references = params->presReferences;
    uint32_t       numRefIdx  = CODEC_MAX_NUM_REF_FRAME;
    uint32_t       step       = 1;
    for (uint32_t i = 0; i < numRefIdx; i++)
    {
        if (references[i] != nullptr && references[i]->pGmmResInfo != nullptr)
        {
            MOS_SURFACE details;
            MOS_ZeroMemory(&details, sizeof(details));
            details.Format = Format_Invalid;
            MHW_MI_CHK_STATUS(m_osInterface->pfnGetResourceInfo(m_osInterface, references[i], &details));

            MOS_MEMCOMP_STATE mmcMode = MOS_MEMCOMP_DISABLED;
            if (params->bMmcEnabled)
            {
                MHW_MI_CHK_STATUS(m_osInterface->pfnGetMemoryCompressionMode(
                    m_osInterface, references[i], &mmcMode));
            }
            else
            {
                mmcMode = MOS_MEMCOMP_DISABLED;
            }

            if (mmcMode == MOS_MEMCOMP_RC || mmcMode == MOS_MEMCOMP_MC)
            {
                cmd->DW61.Value |= (MHW_MEDIA_MEMCOMP_ENABLED << (i * 2 * step)) | ((mmcMode == MOS_MEMCOMP_RC) << (i * 2 * step + 1));
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxMfxInterfaceXe_Xpm::AddMfxEncodeAvcSlice(
    PMOS_COMMAND_BUFFER        cmdBuffer,
    PMHW_BATCH_BUFFER          batchBuffer,
    PMHW_VDBOX_AVC_SLICE_STATE avcSliceState)
{
    mhw_vdbox_mfx_xe_xpm::MFX_AVC_SLICE_STATE_CMD *cmd = nullptr;

    if (cmdBuffer != nullptr)
    {
        MOS_OS_CHK_NULL_RETURN(cmdBuffer->pCmdPtr);
        cmd = (mhw_vdbox_mfx_xe_xpm::MFX_AVC_SLICE_STATE_CMD *)(cmdBuffer->pCmdPtr);
    }
    else if (batchBuffer != nullptr)
    {
        MOS_OS_CHK_NULL_RETURN(batchBuffer->pData);
        cmd = (mhw_vdbox_mfx_xe_xpm::MFX_AVC_SLICE_STATE_CMD *)(batchBuffer->pData + batchBuffer->iCurrent);
    }
    else
    {
        MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MHW_CHK_STATUS_RETURN(MhwVdboxMfxInterfaceG12::AddMfxEncodeAvcSlice(cmdBuffer, batchBuffer, avcSliceState));

    // Inplace patching of the added command in the CMD or BB should be added here
    cmd->DW6.Cabaczerowordinsertionenable = 0;

    return MOS_STATUS_SUCCESS;
}
