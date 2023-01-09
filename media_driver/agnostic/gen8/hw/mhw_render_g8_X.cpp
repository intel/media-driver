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
//! \file     mhw_render_g8_X.cpp
//! \brief    Constructs render engine commands on Gen8-based platforms
//! \details  Each client facing function both creates a HW command and adds
//!           that command to a command or batch buffer.
//!

#include "mhw_render_g8_X.h"
#include "mhw_render_hwcmd_g8_X.h"
#include "mhw_mmio_g8.h"

static const uint32_t l3CacheSqc1RegisterOffset = L3_CACHE_SQC1_REG_OFFSET_G8;
static const uint32_t l3CacheSqc1RegisterValue = L3_CACHE_SQC1_REG_VALUE_G8;

//! \brief      for BDW GT2 with WA for D0 hang
//!              SLM     URB     DC      RO      Rest
//!              256     128      0       0      384 (KB chunks based on GT2)
static const uint32_t l3CacheCntlRegisterValueAllocateSlmD0Wa = L3_CACHE_CNTL_REG_VALUE_ALLOCATE_SLM_D0WA_G8;

MOS_STATUS MhwRenderInterfaceG8::AddMediaVfeCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_VFE_PARAMS                 params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_render_g8_X::MEDIA_VFE_STATE_CMD *cmd =
        (mhw_render_g8_X::MEDIA_VFE_STATE_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwRenderInterfaceGeneric<mhw_render_g8_X>::AddMediaVfeCmd(cmdBuffer, params));

    MHW_MI_CHK_NULL(cmd);
    cmd->DW4.SliceDisable = params->eVfeSliceDisable;

    cmd->DW6.ScoreboardType = params->Scoreboard.ScoreboardType;
    cmd->DW6.ScoreboardMask = params->Scoreboard.ScoreboardMask;
    cmd->DW6.ScoreboardEnable = params->Scoreboard.ScoreboardEnable;

    cmd->DW7.Scoreboard0DeltaX = params->Scoreboard.ScoreboardDelta[0].x;
    cmd->DW7.Scoreboard0DeltaY = params->Scoreboard.ScoreboardDelta[0].y;
    cmd->DW7.Scoreboard1DeltaX = params->Scoreboard.ScoreboardDelta[1].x;
    cmd->DW7.Scoreboard1DeltaY = params->Scoreboard.ScoreboardDelta[1].y;
    cmd->DW7.Scoreboard2DeltaX = params->Scoreboard.ScoreboardDelta[2].x;
    cmd->DW7.Scoreboard2DeltaY = params->Scoreboard.ScoreboardDelta[2].y;
    cmd->DW7.Scoreboard3DeltaX = params->Scoreboard.ScoreboardDelta[3].x;
    cmd->DW7.Scoreboard3DeltaY = params->Scoreboard.ScoreboardDelta[3].y;

    cmd->DW8.Scoreboard4DeltaX = params->Scoreboard.ScoreboardDelta[4].x;
    cmd->DW8.Scoreboard4DeltaY = params->Scoreboard.ScoreboardDelta[4].y;
    cmd->DW8.Scoreboard5DeltaX = params->Scoreboard.ScoreboardDelta[5].x;
    cmd->DW8.Scoreboard5DeltaY = params->Scoreboard.ScoreboardDelta[5].y;
    cmd->DW8.Scoreboard6DeltaX = params->Scoreboard.ScoreboardDelta[6].x;
    cmd->DW8.Scoreboard6DeltaY = params->Scoreboard.ScoreboardDelta[6].y;
    cmd->DW8.Scoreboard7DeltaX = params->Scoreboard.ScoreboardDelta[7].x;
    cmd->DW8.Scoreboard7DeltaY = params->Scoreboard.ScoreboardDelta[7].y;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG8::AddMediaObjectWalkerCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_WALKER_PARAMS              params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(params);

    mhw_render_g8_X::MEDIA_OBJECT_WALKER_CMD *cmd =
        (mhw_render_g8_X::MEDIA_OBJECT_WALKER_CMD*)cmdBuffer->pCmdPtr;

    MHW_MI_CHK_STATUS(MhwRenderInterfaceGeneric<mhw_render_g8_X>::AddMediaObjectWalkerCmd(cmdBuffer, params));

    MHW_MI_CHK_NULL(cmd);
    cmd->DW2.UseScoreboard     = params->UseScoreboard;
    cmd->DW5.ScoreboardMask    = params->ScoreboardMask;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG8::AddPaletteLoadCmd(
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
        mhw_render_g8_X::_3DSTATE_SAMPLER_PALETTE_LOAD0_CMD cmd;
        // Set size of palette load command
        cmd.DW0.DwordLength = params->iNumEntries - 1;
        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    }
    else if (params->iPaletteID == 1)
    {
        mhw_render_g8_X::_3DSTATE_SAMPLER_PALETTE_LOAD1_CMD cmd;
        // Set size of palette load command
        cmd.DW0.DwordLength = params->iNumEntries - 1;
        MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));
    }
    else
    {
        MHW_ASSERTMESSAGE("Invalid palette ID specified: %d.", params->iPaletteID);
        return MOS_STATUS_INVALID_PARAMETER;
    }

    mhw_render_g8_X::PALETTE_ENTRY_CMD entry;
    uint32_t cmdSize = entry.byteSize * params->iNumEntries;

    // Send palette load command followed by palette data
    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, params->pPaletteData, cmdSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG8::AddMediaObject(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMHW_BATCH_BUFFER               batchBuffer,
    PMHW_MEDIA_OBJECT_PARAMS        params)
{
    MHW_FUNCTION_ENTER;

    MHW_MI_CHK_NULL(params);

    mhw_render_g8_X::MEDIA_OBJECT_CMD *cmd;
    if (cmdBuffer)
    {
        cmd = (mhw_render_g8_X::MEDIA_OBJECT_CMD*)cmdBuffer->pCmdPtr;
    }
    else if (batchBuffer)
    {
        cmd = (mhw_render_g8_X::MEDIA_OBJECT_CMD*)(batchBuffer->pData + batchBuffer->iCurrent);
    }
    else
    {
        MHW_ASSERTMESSAGE("No valid buffer to add the command to!");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MHW_MI_CHK_STATUS(MhwRenderInterfaceGeneric<mhw_render_g8_X>::AddMediaObject(cmdBuffer, batchBuffer, params));

    MHW_MI_CHK_NULL(cmd);
    cmd->DW2.UseScoreboard     = params->VfeScoreboard.ScoreboardEnable;
    cmd->DW4.ScoreboardX       = params->VfeScoreboard.Value[0];
    cmd->DW4.ScoredboardY      = params->VfeScoreboard.Value[1];
    cmd->DW5.ScoreboardMask    = params->VfeScoreboard.ScoreboardMask;
    cmd->DW5.ScoreboardColor   = params->VfeScoreboard.ScoreboardColor;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG8::AddGpgpuCsrBaseAddrCmd(
    PMOS_COMMAND_BUFFER             cmdBuffer,
    PMOS_RESOURCE                   csrResource)
{
    MHW_MI_CHK_NULL(m_osInterface);
    MHW_MI_CHK_NULL(cmdBuffer);
    MHW_MI_CHK_NULL(csrResource);

#if (!EMUL)
    MHW_NORMALMESSAGE("GPGPU_CSR_BASE_ADDRESS not supported.");
    return MOS_STATUS_SUCCESS;
#endif

    mhw_render_g8_X::GPGPU_CSR_BASE_ADDRESS_CMD cmd;
    MHW_RESOURCE_PARAMS resourceParams;
    MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
    resourceParams.presResource     = csrResource;
    resourceParams.pdwCmd           = cmd.DW1_2.Value;
    resourceParams.dwLocationInCmd  = 1;

    MHW_MI_CHK_STATUS(AddResourceToCmd(
        m_osInterface,
        cmdBuffer,
        &resourceParams));

    MHW_MI_CHK_STATUS(m_osInterface->pfnAddCommand(cmdBuffer, &cmd, cmd.byteSize));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG8::EnableL3Caching(
    PMHW_RENDER_ENGINE_L3_CACHE_SETTINGS    cacheSettings)
{
    m_l3CacheConfig.bL3CachingEnabled = true;

    m_l3CacheConfig.dwL3CacheSqcReg1_Register = l3CacheSqc1RegisterOffset;
    m_l3CacheConfig.dwL3CacheCntlReg_Register = m_l3CacheCntlRegisterOffset;

    if ( cacheSettings )
    {
        //just overwrites these two values as only these two are used on Gen8
        m_l3CacheConfig.dwL3CacheSqcReg1_Setting = cacheSettings->dwSqcReg1;
        m_l3CacheConfig.dwL3CacheCntlReg_Setting = cacheSettings->dwCntlReg;
        return MOS_STATUS_SUCCESS;
    }

    // default configurations
    m_l3CacheConfig.dwL3CacheSqcReg1_Setting = l3CacheSqc1RegisterValue;

    auto waTable = m_osInterface->pfnGetWaTable(m_osInterface);
    MHW_MI_CHK_NULL(waTable);

    m_l3CacheConfig.dwL3CacheCntlReg_Setting = m_l3CacheCntlRegisterValueDefault;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS MhwRenderInterfaceG8::SetL3Cache(
    PMOS_COMMAND_BUFFER             cmdBuffer )
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    MHW_MI_CHK_NULL( cmdBuffer );

    if ( m_l3CacheConfig.bL3CachingEnabled )
    {
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterParams;
        MOS_ZeroMemory( &loadRegisterParams, sizeof( loadRegisterParams ) );
        loadRegisterParams.dwRegister = m_l3CacheConfig.dwL3CacheCntlReg_Register;
        loadRegisterParams.dwData = m_l3CacheConfig.dwL3CacheCntlReg_Setting;
        MHW_MI_CHK_STATUS( m_miInterface->AddMiLoadRegisterImmCmd( cmdBuffer, &loadRegisterParams ) );
    }

    return eStatus;
}

void MhwRenderInterfaceG8::InitMmioRegisters()
{
    MHW_MI_MMIOREGISTERS *mmioRegisters = &m_mmioRegisters;
    mmioRegisters->generalPurposeRegister0LoOffset  = CS_GENERAL_PURPOSE_REGISTER0_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister0HiOffset  = CS_GENERAL_PURPOSE_REGISTER0_HI_OFFSET_G8;
    mmioRegisters->generalPurposeRegister4LoOffset  = CS_GENERAL_PURPOSE_REGISTER4_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister4HiOffset  = CS_GENERAL_PURPOSE_REGISTER4_HI_OFFSET_G8;
    mmioRegisters->generalPurposeRegister11LoOffset = CS_GENERAL_PURPOSE_REGISTER11_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister11HiOffset = CS_GENERAL_PURPOSE_REGISTER11_HI_OFFSET_G8;
    mmioRegisters->generalPurposeRegister12LoOffset = CS_GENERAL_PURPOSE_REGISTER12_LO_OFFSET_G8;
    mmioRegisters->generalPurposeRegister12HiOffset = CS_GENERAL_PURPOSE_REGISTER12_HI_OFFSET_G8;
}

