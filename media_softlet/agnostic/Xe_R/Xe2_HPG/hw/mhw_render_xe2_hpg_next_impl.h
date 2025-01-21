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
//! \file     mhw_render_xe2_hpg_next_impl.h
//! \brief    MHW Render interface common base for Xe2_HPG
//! \details
//!

#ifndef __MHW_RENDER_XE2_HPG_NEXT_IMPL_H__
#define __MHW_RENDER_XE2_HPG_NEXT_IMPL_H__

#include "mhw_render_impl.h"
#include "mhw_render_hwcmd_xe2_hpg_next.h"
#include "mhw_render_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace render
{
namespace xe2_hpg_next
{
class Impl : public render::Impl<mhw::render::xe2_hpg_next::Cmd>
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
            MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *cacheSettingsHpg = (MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *)cacheSettings;
            m_l3CacheConfig.dwL3CacheAllocReg_Setting  = cacheSettingsHpg->dwAllocReg;
            m_l3CacheConfig.dwL3CacheTcCntlReg_Setting = cacheSettingsHpg->dwTcCntlReg;
            // update default settings is needed from CM HAL call
            if (cacheSettingsHpg->bUpdateDefault)
            {
                m_l3CacheAllocRegisterValueDefault  = cacheSettingsHpg->dwAllocReg;
                m_l3CacheTcCntlRegisterValueDefault = cacheSettingsHpg->dwTcCntlReg;
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
        cmd.DW1.Mask1            = 0xFFFF;
        cmd.DW1.LargeGrfMode     = 0;
        cmd.DW1.EuThreadSchedulingModeOverride = params.forceEuThreadSchedulingMode;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(CFE_STATE)
    {
        _MHW_SETCMD_CALLBASE(CFE_STATE);

        cmd.DW3.MaximumNumberOfThreads     = params.dwMaximumNumberofThreads;
        cmd.DW1.ScratchSpaceBuffer         = params.ScratchSpaceBuffer;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(COMPUTE_WALKER)
    {
        _MHW_SETCMD_CALLBASE(COMPUTE_WALKER);

        cmd.DW2.IndirectDataLength       = params.IndirectDataLength;
        cmd.DW3.IndirectDataStartAddress = params.IndirectDataStartAddress >> MHW_COMPUTE_INDIRECT_SHIFT;

        if (params.simdSize == 16)
        {
            cmd.DW4.SIMDSize      = 1;
            cmd.DW4.MessageSIMD   = Cmd::COMPUTE_WALKER_CMD::MESSAGE_SIMD_SIMT16;
            cmd.DW5.ExecutionMask = 0xffff;
        }
        else
        {
            cmd.DW4.SIMDSize      = 2;
            cmd.DW4.MessageSIMD   = Cmd::COMPUTE_WALKER_CMD::MESSAGE_SIMD_SIMT32;
            cmd.DW5.ExecutionMask = 0xffffffff;
        }
        cmd.DW6.LocalXMaximum = params.ThreadWidth - 1;
        cmd.DW6.LocalYMaximum = params.ThreadHeight - 1;
        cmd.DW6.LocalZMaximum = params.ThreadDepth - 1;

        cmd.DW7.ThreadGroupIDXDimension = params.GroupWidth;
        cmd.DW8.ThreadGroupIDYDimension = params.GroupHeight;
        cmd.DW9.ThreadGroupIDZDimension = params.GroupDepth;
        cmd.DW10.ThreadGroupIDStartingX = params.GroupStartingX;
        cmd.DW11.ThreadGroupIDStartingY = params.GroupStartingY;
        cmd.DW12.ThreadGroupIDStartingZ = params.GroupStartingZ;

        cmd.InterfaceDescriptor.DW0.KernelStartPointer                = params.dwKernelOffset >> MHW_KERNEL_OFFSET_SHIFT;
        cmd.InterfaceDescriptor.DW3.SamplerCount                      = params.dwSamplerCount;
        cmd.InterfaceDescriptor.DW3.SamplerStatePointer               = params.dwSamplerOffset >> MHW_SAMPLER_SHIFT;
        cmd.InterfaceDescriptor.DW4.BindingTablePointer               = MOS_ROUNDUP_SHIFT(params.dwBindingTableOffset, MHW_BINDING_TABLE_ID_SHIFT);
        cmd.InterfaceDescriptor.DW5.NumberOfThreadsInGpgpuThreadGroup = params.dwNumberofThreadsInGPGPUGroup;
        cmd.InterfaceDescriptor.DW5.SharedLocalMemorySize             = params.dwSharedLocalMemorySize;

        cmd.InterfaceDescriptor.DW7.PreferredSlmAllocationSizePerSubslice = params.preferredSlmAllocationSize;

        // when Barriers is not 0, the EU fusion will close.
        // Assigns barrier count.
        if (params.bBarrierEnable)
        {  // Bits [28:30] represent the number of barriers on DG2.
            cmd.InterfaceDescriptor.DW5.NumberOfBarriers = 1;
        }

        cmd.DW4.EmitInlineParameter = params.isEmitInlineParameter;
        MOS_SecureMemcpy(cmd.InlineData.Value, sizeof(cmd.InlineData.Value), params.inlineData, params.inlineDataLength);

        cmd.DW4.GenerateLocalId = params.isGenerateLocalId;
        cmd.DW4.EmitLocal       = params.emitLocal;

        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t                                                          = render::Impl<mhw::render::xe2_hpg_next::Cmd>;
    uint32_t                          m_l3CacheTcCntlRegisterValueDefault = 0x80000080;
    uint32_t                          m_l3CacheAllocRegisterValueDefault  = 0xD0000020;
MEDIA_CLASS_DEFINE_END(mhw__render__xe2_hpg_next__Impl)
};

}  // namespace xe2_hpg_next
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_XE_HPG_IMPL_H__