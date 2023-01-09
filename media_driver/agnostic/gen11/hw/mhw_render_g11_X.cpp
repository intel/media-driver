/*
* Copyright (c) 2015-2017, Intel Corporation
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
//! \file     mhw_render_g11_X.cpp
//! \brief    Constructs render engine commands on Gen11-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_render_g11_X.h"
#include "mhw_render_hwcmd_g11_X.h"
#include "mhw_mmio_g11.h"

MOS_STATUS MhwRenderInterfaceG11::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VFE_PARAMS                 params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);
    MHW_MI_CHK_NULL(params);

    mhw_render_g11_X::MEDIA_VFE_STATE_CMD *cmd =
        (mhw_render_g11_X::MEDIA_VFE_STATE_CMD*)cmdBuffer->pCmdPtr;
    
    MHW_MI_CHK_STATUS(MhwRenderInterfaceGeneric<mhw_render_g11_X>::AddMediaVfeCmd(cmdBuffer, params));

    MHW_MI_CHK_NULL(cmd);
    cmd->DW4.MaximumNumberOfDualSubslices = params->eVfeSliceDisable;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG11::AddPipelineSelectCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    bool                            gpGpuPipe)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(cmdBuffer->pCmdPtr);

    mhw_render_g11_X::PIPELINE_SELECT_CMD *cmd =
        (mhw_render_g11_X::PIPELINE_SELECT_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwRenderInterfaceGeneric<mhw_render_g11_X>::AddPipelineSelectCmd(cmdBuffer, gpGpuPipe));

    MHW_MI_CHK_NULL(cmd);
    cmd->DW0.MaskBits = 0x13;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG11::AddMediaObject(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    PMHW_MEDIA_OBJECT_PARAMS        params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_render_g11_X::MEDIA_OBJECT_CMD *cmd;
    if (cmdBuffer)
    {
        cmd = (mhw_render_g11_X::MEDIA_OBJECT_CMD*)cmdBuffer->pCmdPtr;
    }
    else if (batchBuffer)
    {
        cmd = (mhw_render_g11_X::MEDIA_OBJECT_CMD*)(batchBuffer->pData + batchBuffer->iCurrent);
    }
    else
    {
        MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MHW_MI_CHK_STATUS(MhwRenderInterfaceGeneric<mhw_render_g11_X>::AddMediaObject(cmdBuffer, batchBuffer, params));

    MHW_MI_CHK_NULL(cmd);
    cmd->DW4.XPosition = params->VfeScoreboard.Value[0];
    cmd->DW4.YPosition = params->VfeScoreboard.Value[1];

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG11::AddPaletteLoadCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_PALETTE_PARAMS             params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);
    MHW_MI_CHK_NULL(params->pPaletteData);

    if (params->iNumEntries <= 0)
    {
        MHW_ASSERTMESSAGE("Invalid number of palette entries.");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Send Palettes in use
    if (params->iPaletteID == 0)
    {
        mhw_render_g11_X::_3DSTATE_SAMPLER_PALETTE_LOAD0_CMD cmd;
        // Set size of palette load command
        cmd.DW0.DwordLength = params->iNumEntries - 1;
        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    }
    else if (params->iPaletteID == 1)
    {
        mhw_render_g11_X::_3DSTATE_SAMPLER_PALETTE_LOAD1_CMD cmd;
        // Set size of palette load command
        cmd.DW0.DwordLength = params->iNumEntries - 1;
        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid palette ID specified: %d.", params->iPaletteID);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    mhw_render_g11_X::PALETTE_ENTRY_CMD entry;
    uint32_t cmdSize = entry.byteSize * params->iNumEntries;

    // Send palette load command followed by palette data    
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, params->pPaletteData, cmdSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG11::EnableL3Caching(
    PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS    cacheSettings )
{
    // L3 Caching enabled by default
    m_l3CacheConfig.bL3CachingEnabled = true;
    m_l3CacheConfig.dwL3CacheCntlReg_Register = m_l3CacheCntlRegisterOffset;
    m_l3CacheConfig.dwL3CacheTcCntlReg_Register = m_l3CacheTcCntlRegisterOffset;

    if ( cacheSettings )
    {
        PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G11 cacheSettingsG11 = dynamic_cast< PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS_G11 >( cacheSettings );
        if (cacheSettingsG11 == nullptr)
        {
            MHW_ASSERTMESSAGE("Gen11-Specific Params are needed.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
        m_l3CacheConfig.dwL3CacheCntlReg_Setting = cacheSettingsG11->dwCntlReg;
        m_l3CacheConfig.dwL3CacheTcCntlReg_Setting = cacheSettingsG11->dwTcCntlReg;
    }
    else // Use the default setting if regkey is not set
    {
        m_l3CacheConfig.dwL3CacheCntlReg_Setting = m_l3CacheCntlRegisterValueDefault;
        m_l3CacheConfig.dwL3CacheTcCntlReg_Setting = m_l3CacheTcCntlRegisterValueDefault;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG11::SetL3Cache( PMOS_COMMAND_BUFFER cmdBuffer )
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL( cmdBuffer );

    if ( m_l3CacheConfig.bL3CachingEnabled )
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterParams;

        if ( m_l3CacheConfig.dwL3CacheCntlReg_Setting != 0 )
        {
            MOS_ZeroMemory( &loadRegisterParams, sizeof( loadRegisterParams ) );
            loadRegisterParams.dwRegister = m_l3CacheConfig.dwL3CacheCntlReg_Register;
            loadRegisterParams.dwData = m_l3CacheConfig.dwL3CacheCntlReg_Setting;
            MHW_MI_CHK_STATUS( m_miInterface->AddMiLoadRegisterImmCmd( cmdBuffer, &loadRegisterParams ) );
        }

        if ( m_l3CacheConfig.dwL3CacheTcCntlReg_Setting != 0 )
        {
            MOS_ZeroMemory( &loadRegisterParams, sizeof( loadRegisterParams ) );
            loadRegisterParams.dwRegister = m_l3CacheConfig.dwL3CacheTcCntlReg_Register;
            loadRegisterParams.dwData = m_l3CacheConfig.dwL3CacheTcCntlReg_Setting;
            MHW_MI_CHK_STATUS( m_miInterface->AddMiLoadRegisterImmCmd( cmdBuffer, &loadRegisterParams ) );
        }
    }

    return eStatus;
}

MOS_STATUS MhwRenderInterfaceG11::AddGpgpuCsrBaseAddrCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMOS_RESOURCE                   csrResource)
{
    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(csrResource);

#if (EMUL)
    MHW_NORMALMESSAGE("GPGPU_CSR_BASE_ADDRESS not supported.");
    return MOS_STATUS_SUCCESS;
#endif

    mhw_render_g11_X::STATE_CSR_BASE_ADDRESS_CMD cmd;
    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.presResource = csrResource;
    resourceParams.pdwCmd = (uint32_t *)cmd.DW1_2.Value;
    resourceParams.dwLocationInCmd = 1;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

void MhwRenderInterfaceG11::InitMmioRegisters()
{
    MHW_MI_MMIOREGISTERS *mmioRegisters = &m_mmioRegisters;
    mmioRegisters->generalPurposeRegister0LoOffset  = CS_GENERAL_PURPOSE_REGISTER0_LO_OFFSET_G11;
    mmioRegisters->generalPurposeRegister0HiOffset  = CS_GENERAL_PURPOSE_REGISTER0_HI_OFFSET_G11;
    mmioRegisters->generalPurposeRegister4LoOffset  = CS_GENERAL_PURPOSE_REGISTER4_LO_OFFSET_G11;
    mmioRegisters->generalPurposeRegister4HiOffset  = CS_GENERAL_PURPOSE_REGISTER4_HI_OFFSET_G11;
    mmioRegisters->generalPurposeRegister11LoOffset = CS_GENERAL_PURPOSE_REGISTER11_LO_OFFSET_G11;
    mmioRegisters->generalPurposeRegister11HiOffset = CS_GENERAL_PURPOSE_REGISTER11_HI_OFFSET_G11;
    mmioRegisters->generalPurposeRegister12LoOffset = CS_GENERAL_PURPOSE_REGISTER12_LO_OFFSET_G11;
    mmioRegisters->generalPurposeRegister12HiOffset = CS_GENERAL_PURPOSE_REGISTER12_HI_OFFSET_G11;
}
