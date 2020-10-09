/*
* Copyright (c) 2017-2019, Intel Corporation
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
//! \file     mhw_vdbox_huc_g12_X.cpp
//! \brief    Constructs VdBox Huc commands on Gen12-based platforms

#include "mhw_vdbox_huc_g12_X.h"
#include "mhw_vdbox_vdenc_hwcmd_g12_X.h"
#include "mhw_mi.h"
#include "mhw_mmio_g12.h"

void MhwVdboxHucInterfaceG12::InitMmioRegisters()
{
    MmioRegistersHuc *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

    mmioRegisters->hucUKernelHdrInfoRegOffset = HUC_UKERNEL_HDR_INFO_REG_OFFSET_NODE_1_INIT_G12;
    mmioRegisters->hucStatusRegOffset         = HUC_STATUS_REG_OFFSET_NODE_1_INIT_G12;
    mmioRegisters->hucStatus2RegOffset        = HUC_STATUS2_REG_OFFSET_NODE_1_INIT_G12;

    m_mmioRegisters[MHW_VDBOX_NODE_2] = m_mmioRegisters[MHW_VDBOX_NODE_1];
}

MOS_STATUS MhwVdboxHucInterfaceG12::GetHucStateCommandSize(
    uint32_t                        mode,
    uint32_t                        *commandsSize,
    uint32_t                        *patchListSize,
    PMHW_VDBOX_STATE_CMDSIZE_PARAMS params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(commandsSize);
    MHW_MI_CHK_NULL(patchListSize);

    uint32_t    maxSize = *commandsSize;
    uint32_t    patchListMaxSize = *patchListSize;

    MHW_MI_CHK_STATUS((MhwVdboxHucInterfaceGeneric<mhw_vdbox_huc_g12_X, mhw_mi_g12_X>::
        GetHucStateCommandSize(mode, commandsSize, patchListSize, params)));

    maxSize += mhw_vdbox_vdenc_g12_X::VD_PIPELINE_FLUSH_CMD::byteSize;
    patchListMaxSize += PATCH_LIST_COMMAND(VD_PIPELINE_FLUSH_CMD);

    *commandsSize  = maxSize;
    *patchListSize = patchListMaxSize;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxHucInterfaceG12::AddHucPipeModeSelectCmd(
    MOS_COMMAND_BUFFER                  *cmdBuffer,
    MHW_VDBOX_PIPE_MODE_SELECT_PARAMS   *params)
{

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    //for gen 11, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
    MHW_MI_CHK_STATUS(m_MiInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    mhw_vdbox_huc_g12_X::HUC_PIPE_MODE_SELECT_CMD       cmd;

    if (!params->disableProtectionSetting)
    {
        MHW_MI_CHK_STATUS(m_cpInterface->SetProtectionSettingsForHucPipeModeSelect((uint32_t *)&cmd));
    }

    cmd.DW1.IndirectStreamOutEnable = params->bStreamOutEnabled;
    cmd.DW2.MediaSoftResetCounterPer1000Clocks = params->dwMediaSoftResetCounterValue;

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    //for gen 11, we need to add MFX wait for both KIN and VRT before and after HUC Pipemode select...
    MHW_MI_CHK_STATUS(m_MiInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwVdboxHucInterfaceG12::AddHucImemStateCmd(
    MOS_COMMAND_BUFFER                  *cmdBuffer,
    MHW_VDBOX_HUC_IMEM_STATE_PARAMS     *params)
{
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_vdbox_huc_g12_X::HUC_IMEM_STATE_CMD cmd;

    cmd.DW4.HucFirmwareDescriptor = params->dwKernelDescriptor;

    MHW_MI_CHK_STATUS(Mos_AddCommand(cmdBuffer, &cmd, cmd.byteSize));

    MHW_MI_CHK_STATUS(m_MiInterface->AddMfxWaitCmd(cmdBuffer, nullptr, true));

    return MOS_STATUS_SUCCESS;
}

