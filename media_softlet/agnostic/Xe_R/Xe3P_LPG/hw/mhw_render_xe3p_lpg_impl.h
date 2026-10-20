/*
* Copyright (c) 2023-2024, Intel Corporation
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
//! \file     mhw_render_xe3p_lpg_impl.h
//! \brief    MHW Render interface common base for Xe3P_LPG
//! \details
//!

#ifndef __MHW_RENDER_XE3P_LPG_IMPL_H__
#define __MHW_RENDER_XE3P_LPG_IMPL_H__

#include "mhw_render_impl.h"
#include "mhw_render_hwcmd_xe3p_lpg.h"
#include "mhw_render_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace render
{
namespace xe3p_lpg
{
class Impl : public render::Impl<mhw::render::xe3p_lpg::Cmd>
{
public:
    Impl(PMOS_INTERFACE osItf) : base_t(osItf)
    {
        MHW_FUNCTION_ENTER;

        InitMmioRegisters();
    };

    MOS_STATUS InitMmioRegisters() override
    {
        MHW_FUNCTION_ENTER;
        MHW_MI_MMIOREGISTERS *mmioRegisters             = &m_mmioRegisters;
        mmioRegisters->generalPurposeRegister0LoOffset  = CS_GENERAL_PURPOSE_REGISTER0_LO_OFFSET;
        mmioRegisters->generalPurposeRegister0HiOffset  = CS_GENERAL_PURPOSE_REGISTER0_HI_OFFSET;
        mmioRegisters->generalPurposeRegister4LoOffset  = CS_GENERAL_PURPOSE_REGISTER4_LO_OFFSET;
        mmioRegisters->generalPurposeRegister4HiOffset  = CS_GENERAL_PURPOSE_REGISTER4_HI_OFFSET;
        mmioRegisters->generalPurposeRegister11LoOffset = CS_GENERAL_PURPOSE_REGISTER11_LO_OFFSET;
        mmioRegisters->generalPurposeRegister11HiOffset = CS_GENERAL_PURPOSE_REGISTER11_HI_OFFSET;
        mmioRegisters->generalPurposeRegister12LoOffset = CS_GENERAL_PURPOSE_REGISTER12_LO_OFFSET;
        mmioRegisters->generalPurposeRegister12HiOffset = CS_GENERAL_PURPOSE_REGISTER12_HI_OFFSET;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EnableL3Caching(mhw::render::MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *cacheSettings) override
    {
        MHW_FUNCTION_ENTER;
        // L3 Caching enabled by default
        m_l3CacheConfig.bL3CachingEnabled               = true;
        m_l3CacheConfig.dwRcsL3CacheAllocReg_Register   = M_MMIO_RCS_L3ALLOCREG;
        m_l3CacheConfig.dwRcsL3CacheTcCntlReg_Register  = M_MMIO_RCS_TCCNTLREG;
        m_l3CacheConfig.dwCcs0L3CacheAllocReg_Register  = M_MMIO_CCS0_L3ALLOCREG;
        m_l3CacheConfig.dwCcs0L3CacheTcCntlReg_Register = M_MMIO_CCS0_TCCNTLREG;
        if (cacheSettings)
        {
            MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *cacheSettingsLpg = (MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *)cacheSettings;
            m_l3CacheConfig.dwL3CacheAllocReg_Setting  = cacheSettingsLpg->dwAllocReg;
            m_l3CacheConfig.dwL3CacheTcCntlReg_Setting = cacheSettingsLpg->dwTcCntlReg;
            // update default settings is needed from CM HAL call
            if (cacheSettingsLpg->bUpdateDefault)
            {
                m_l3CacheAllocRegisterValueDefault  = cacheSettingsLpg->dwAllocReg;
                m_l3CacheTcCntlRegisterValueDefault = cacheSettingsLpg->dwTcCntlReg;
            }
        }
        else  // Use the default setting if regkey is not set
        {
            // different default settings after CM HAL call
            m_l3CacheConfig.dwL3CacheAllocReg_Setting  = m_l3CacheAllocRegisterValueDefault;
            m_l3CacheConfig.dwL3CacheTcCntlReg_Setting = m_l3CacheTcCntlRegisterValueDefault;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetL3Cache(PMOS_COMMAND_BUFFER cmdBuffer, std::shared_ptr<mhw::mi::Itf> miItf) override
    {
        MHW_FUNCTION_ENTER;
        MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(miItf);

        if (m_l3CacheConfig.bL3CachingEnabled)
        {
            //L3CacheAllocReg_Setting and L3CacheTcCntlReg_Setting
            if ((m_l3CacheConfig.dwL3CacheAllocReg_Setting != 0) || (m_l3CacheConfig.dwL3CacheTcCntlReg_Setting != 0))
            {
                //update L3 AllocReg setting for RCS; CCS L3 AllocReg setting will be dulicated from RCS
                auto& l3CachePar = miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
                l3CachePar = {};
                l3CachePar.dwRegister = m_l3CacheConfig.dwRcsL3CacheAllocReg_Register;
                l3CachePar.dwData     = m_l3CacheConfig.dwL3CacheAllocReg_Setting;
                MHW_MI_CHK_STATUS(miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

                //update L3 TcCntlReg setting for RCS; CCS L3 TcCntlReg setting will be dulicated from RCS
                auto& rcsL3CacheTcCntlPar = miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
                rcsL3CacheTcCntlPar = {};
                rcsL3CacheTcCntlPar.dwRegister = m_l3CacheConfig.dwRcsL3CacheTcCntlReg_Register;
                rcsL3CacheTcCntlPar.dwData     = m_l3CacheConfig.dwL3CacheTcCntlReg_Setting;
                MHW_MI_CHK_STATUS(miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));
            }
        }

        return eStatus;
    }

    MOS_STATUS SetupInlineData() override
    {
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(_3DSTATE_CHROMA_KEY)
    {
        _MHW_SETCMD_CALLBASE(_3DSTATE_CHROMA_KEY);

        cmd.DW1.ChromakeyTableIndex = params.dwIndex;
        cmd.DW2.ChromakeyLowValue   = params.dwLow;
        cmd.DW3.ChromakeyHighValue  = params.dwHigh;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(PIPELINE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(PIPELINE_SELECT);
        cmd.DW0.MaskBits = 0x13;
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(STATE_COMPUTE_MODE)
    {
        _MHW_SETCMD_CALLBASE(STATE_COMPUTE_MODE);
        cmd.DW1.Mask1                          = 0xFFFF;
        cmd.DW1.LargeGrfMode                   = params.enableLargeGrf;
        cmd.DW1.EuThreadSchedulingModeOverride = params.forceEuThreadSchedulingMode;
        cmd.DW1.EnableVariableRegisterSizeAllocationVrt = Cmd::STATE_COMPUTE_MODE_CMD::ENABLE_VARIABLE_REGISTER_SIZE_ALLOCATIONVRT_ENABLE;
        
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(STATE_BASE_ADDRESS)
    {
        _MHW_SETCMD_CALLBASE(STATE_BASE_ADDRESS);

        // Media uses AOT kernel for technical reasons, and the POR path is the efficient64 path.
        cmd.DW3.BaseaddrDis = Cmd::STATE_BASE_ADDRESS_CMD::BASEADDR_DIS::BASEADDR_DIS_FULL64BMODE;

        // When efficient64 is enabled, BaseAddressModifyEnable does not control the base address
        // But instead it controls the MOCS. The corresponding MOCS value will be written to hardware when ModifyEnable is 1
        // Otherwise, the MOCS value will be discarded.
        cmd.DW16_17.BindlessSurfaceStateBaseAddressModifyEnable  = true;
        cmd.DW19_20.BindlessSamplerStateBaseAddressModifyEnable  = true;
        cmd.DW16_17.BindlessSurfaceStateMemoryObjectControlState = params.mocs4SurfaceState & ~1;
        cmd.DW19_20.BindlessSamplerStateMemoryObjectControlState = params.mocs4SurfaceState & ~1;

        cmd.DW1_2.GeneralStateBaseAddressModifyEnable  = true;
        cmd.DW1_2.GeneralStateMemoryObjectControlState = params.mocs4GeneralState & ~1;

        cmd.DW3.StatelessDataPortAccessMemoryObjectControlState = params.mocs4StatelessDataport & ~1;

        cmd.DW4_5.SurfaceStateBaseAddressModifyEnable  = true;
        cmd.DW4_5.SurfaceStateMemoryObjectControlState = params.mocs4SurfaceState & ~1;

        cmd.DW6_7.DynamicStateBaseAddressModifyEnable  = true;
        cmd.DW6_7.DynamicStateMemoryObjectControlState = params.mocs4DynamicState & ~1;

        cmd.DW10_11.InstructionBaseAddressModifyEnable  = true;
        cmd.DW10_11.InstructionMemoryObjectControlState = params.mocs4InstructionCache & ~1;

        cmd.DW21.BindlessSamplerStateBufferSize = 0xFFFFE;
        cmd.DW18.BindlessSurfaceStateSize       = 0xFFFFFFFE;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(COMPUTE_WALKER)
    {
        _MHW_SETCMD_CALLBASE(COMPUTE_WALKER);

        //cmd.DW0.CfeSubopcode  = 2;
        cmd.DW4.SimdSize      = 2;
        cmd.DW4.MessageSimd   = Cmd::COMPUTE_WALKER_CMD::MESSAGE_SIMD_SIMT32;
        cmd.DW5.ExecutionMask = 0xffffffff;
        cmd.DW6.LocalXMaximum = params.ThreadWidth - 1;
        cmd.DW6.LocalYMaximum = params.ThreadHeight - 1;
        cmd.DW6.LocalZMaximum = params.ThreadDepth - 1;

        cmd.DW7.ThreadGroupIdXDimension = params.GroupWidth;
        cmd.DW8.ThreadGroupIdYDimension = params.GroupHeight;
        cmd.DW9.ThreadGroupIdZDimension = params.GroupDepth;
        cmd.DW10.ThreadGroupIdStartingX = params.GroupStartingX;
        cmd.DW11.ThreadGroupIdStartingY = params.GroupStartingY;
        cmd.DW12.ThreadGroupIdStartingZ = params.GroupStartingZ;

        MHW_RESOURCE_PARAMS kernelInstructParams = {};
        kernelInstructParams.pdwCmd              = cmd.InterfaceDescriptor.DW0_1.Value;
        kernelInstructParams.presResource        = params.heapsResource.presInstructionBuffer;
        kernelInstructParams.dwLocationInCmd     = _MHW_CMD_DW_LOCATION(InterfaceDescriptor.DW0_1.Value);
        kernelInstructParams.dwOffset            = params.dwKernelOffset;
        kernelInstructParams.bIsWritable         = false;
        kernelInstructParams.HwCommandType       = MOS_COMPUTE_WALKER;
        MHW_MI_CHK_STATUS(AddResourceToCmd(m_osItf, m_currentCmdBuf, &kernelInstructParams));

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
        if (m_osItf && m_osItf->pfnAddIndirectState && params.heapsResource.instructionBufferPtr)
        {
            m_osItf->pfnAddIndirectStateByAddressValue(
                m_osItf,
                params.kernelSize,
                (uint32_t *)(params.heapsResource.instructionBufferPtr + params.dwKernelOffset),
                *cmd.InterfaceDescriptor.DW0_1.Value,
                *(cmd.InterfaceDescriptor.DW0_1.Value + 1),
                "KERNEL_INSTRUCTION");
        }
#endif

        cmd.InterfaceDescriptor.DW5.NumberOfThreadsInGpgpuThreadGroup = params.dwNumberofThreadsInGPGPUGroup;
        cmd.InterfaceDescriptor.DW5.SharedLocalMemorySize             = params.dwSharedLocalMemorySize;

        cmd.InterfaceDescriptor.DW7.PreferredSlmAllocationSize        = params.preferredSlmAllocationSize;

        // when Barriers is not 0, the EU fusion will close.
        // Assigns barrier count.
        if (params.bBarrierEnable)
        {  // Bits [28:30] represent the number of barriers.
            cmd.InterfaceDescriptor.DW5.NumberOfBarriers = 1;
        }

        cmd.DW4.EmitInlineParameter = params.isEmitInlineParameter;
        MOS_SecureMemcpy(cmd.InlineData.Value, sizeof(cmd.InlineData.Value), params.inlineData, params.inlineDataLength);

        cmd.DW4.GenerateLocalId = params.isGenerateLocalId;
        cmd.DW4.EmitLocal       = params.emitLocal;

        cmd.DW3.MaximumNumberOfThreads = params.cfeState.dwMaximumNumberofThreads;

        switch (params.registersPerThread)
        {
        case 32:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS32;
            break;
        case 64:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS64;
            break;
        case 96:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS96;
            break;
        case 0:
        case 128:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS128;
            break;
        case 160:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS160;
            break;
        case 192:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS192;
            break;
        case 256:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS256;
            break;
        case 512:
            cmd.InterfaceDescriptor.DW2.RegistersPerThread = Cmd::INTERFACE_DESCRIPTOR_DATA_CMD::REGISTERS_PER_THREAD_REGISTERS512;
            break;
        default:
            MHW_MI_CHK_STATUS(MOS_STATUS_INVALID_PARAMETER);
        }
        

        if (params.heapsResource.curbeResourceListSize > 0)
        {
            MHW_MI_CHK_NULL(params.heapsResource.curbeResourceList);
            for (uint32_t i = 0; i < params.heapsResource.curbeResourceListSize; ++i)
            {
                MHW_INDIRECT_STATE_RESOURCE_PARAMS &resourceParam = params.heapsResource.curbeResourceList[i];
                MHW_MI_CHK_NULL(resourceParam.stateHeap);
                MHW_MI_CHK_NULL(resourceParam.stateBasePtr);
                MOS_COMMAND_BUFFER  cmdBuffer = {};
                MHW_RESOURCE_PARAMS params    = {};
                cmdBuffer.OsResource          = *resourceParam.stateHeap;
                cmdBuffer.pCmdBase            = (uint32_t *)resourceParam.stateBasePtr;
                cmdBuffer.iOffset             = resourceParam.stateOffset;

                params.pdwCmd          = (uint32_t *)(resourceParam.stateBasePtr + resourceParam.stateOffset);
                params.presResource    = resourceParam.resource;
                params.dwLocationInCmd = 0;
                params.dwOffset        = resourceParam.resourceOffset;
                params.bIsWritable     = resourceParam.isWrite;
                params.HwCommandType   = MOS_BINDLESS_STATELESS_SURFACE;

                HalOcaInterfaceNext::InsertResourceHeapToCurrentCmdBufferOcaBufferHandle(cmdBuffer.pCmdBase, m_osItf, m_currentCmdBuf);
                MHW_MI_CHK_STATUS(AddResourceToCmd(m_osItf, &cmdBuffer, &params));

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
                if (m_osItf && m_osItf->pfnAddIndirectState && resourceParam.resourceBasePtr && resourceParam.needDump)
                {
                    m_osItf->pfnAddIndirectState(
                        m_osItf,
                        resourceParam.dumpSize,
                        (uint32_t *)(resourceParam.resourceBasePtr + resourceParam.resourceOffset),
                        (uint32_t *)((uint8_t *)cmdBuffer.pCmdBase + cmdBuffer.iOffset),
                        (uint32_t *)((uint8_t *)cmdBuffer.pCmdBase + cmdBuffer.iOffset) + 1,
                        resourceParam.dumpName);
                }
#endif
            }
        }

        if (params.heapsResource.inlineResourceListSize > 0)
        {
            MHW_MI_CHK_NULL(params.heapsResource.inlineResourceList);
            for (uint32_t i = 0; i < params.heapsResource.inlineResourceListSize; ++i)
            {
                MHW_INDIRECT_STATE_RESOURCE_PARAMS &resourceParam = params.heapsResource.inlineResourceList[i];
                MHW_RESOURCE_PARAMS                 params        = {};

                if (resourceParam.stateOffset % sizeof(uint32_t) != 0)
                {
                    MHW_MI_CHK_STATUS(MOS_STATUS_INVALID_PARAMETER);
                }
                params.pdwCmd          = (uint32_t *)((uint8_t *)cmd.InlineData.Value + resourceParam.stateOffset);
                params.presResource    = resourceParam.resource;
                params.dwLocationInCmd = resourceParam.stateOffset / sizeof(uint32_t) + _MHW_CMD_DW_LOCATION(InlineData.Value);
                params.dwOffset        = resourceParam.resourceOffset;
                params.bIsWritable     = resourceParam.isWrite;
                params.HwCommandType   = MOS_BINDLESS_STATELESS_SURFACE;

                MHW_MI_CHK_STATUS(AddResourceToCmd(m_osItf, m_currentCmdBuf, &params));

#if MOS_COMMAND_BUFFER_DUMP_SUPPORTED
                if (m_osItf && m_osItf->pfnAddIndirectState && resourceParam.resourceBasePtr && resourceParam.needDump)
                {
                    m_osItf->pfnAddIndirectStateByAddressValue(
                        m_osItf,
                        resourceParam.dumpSize,
                        (uint32_t *)(resourceParam.resourceBasePtr + resourceParam.resourceOffset),
                        resourceParam.stateOffset < (sizeof(cmd.InlineData.Value) - 8) ? *(uint32_t *)((uint8_t *)cmd.InlineData.Value + resourceParam.stateOffset) : 0,
                        resourceParam.stateOffset < (sizeof(cmd.InlineData.Value) - 8) ? *((uint32_t *)((uint8_t *)cmd.InlineData.Value + resourceParam.stateOffset) + 1) : 0,
                        resourceParam.dumpName);
                }
#endif
            }
        }

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t                                                          = render::Impl<mhw::render::xe3p_lpg::Cmd>;
    uint32_t                          m_l3CacheTcCntlRegisterValueDefault = 0x80000080;
    uint32_t                          m_l3CacheAllocRegisterValueDefault  = 0xD0000020;
MEDIA_CLASS_DEFINE_END(mhw__render__xe3p_lpg__Impl)
};

}  // namespace xe3p_lpg
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_XE3P_LPG_IMPL_H__