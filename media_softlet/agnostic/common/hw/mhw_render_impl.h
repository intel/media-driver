/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     mhw_render_impl.h
//! \brief    MHW RENDER interface common base
//! \details
//!

#ifndef __MHW_RENDER_IMPL_H__
#define __MHW_RENDER_IMPL_H__

#include "mhw_render_itf.h"
#include "mhw_impl.h"
#include "mhw_mmio.h"

#ifdef IGFX_RENDER_INTERFACE_EXT_SUPPORT
#include "mhw_render_impl_ext.h"
#endif

namespace mhw
{
namespace render
{

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _RENDER_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    MOS_STATUS EnableL3Caching(MHW_RENDER_ENGINE_L3_CACHE_SETTINGS *cacheSettings) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS SetL3Cache(PMOS_COMMAND_BUFFER cmdBuffer, MhwMiInterface* pMhwMiInterface) override
    {
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS InitMmioRegisters() override
    {
        return MOS_STATUS_SUCCESS;
    }

    PMHW_MI_MMIOREGISTERS GetMmioRegisters() override
    {
        return &m_mmioRegisters;
    }

protected:
    using base_t = Itf;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_MP_RESOURCE_USAGE_END] = {};
    MHW_MI_MMIOREGISTERS    m_mmioRegisters = {};
    bool        m_preemptionEnabled = false;
    uint32_t    m_preemptionCntlRegisterOffset = 0;
    uint32_t    m_preemptionCntlRegisterValue = 0;

    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;
        InitPreemption();
    }

    void inline InitMocsParams(
        MHW_RESOURCE_PARAMS& hwResourceParam,
        uint32_t* addr,
        uint8_t             bitFieldLow,
        uint8_t             bitFieldHigh)
    {
        hwResourceParam.mocsParams.mocsTableIndex = addr;
        hwResourceParam.mocsParams.bitFieldLow = bitFieldLow;
        hwResourceParam.mocsParams.bitFieldHigh = bitFieldHigh;
        return;
    }

public:
    void InitPreemption()
    {
        auto skuTable = this->m_osItf->pfnGetSkuTable(this->m_osItf);
        auto waTable = this->m_osItf->pfnGetWaTable(this->m_osItf);

        if (skuTable == nullptr || waTable == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid SKU or WA table acquired");
            return;
        }

        if (MEDIA_IS_SKU(skuTable, FtrMediaThreadGroupLevelPreempt) ||
            MEDIA_IS_SKU(skuTable, FtrMediaMidBatchPreempt))
        {
            m_preemptionEnabled = true;

#if (_DEBUG || _RELEASE_INTERNAL)
            MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
            MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
            MOS_UserFeature_ReadValue_ID(
                nullptr,
                __MEDIA_USER_FEATURE_VALUE_MEDIA_PREEMPTION_ENABLE_ID,
                &UserFeatureData,
                this->m_osItf->pOsContext);
            m_preemptionEnabled = (UserFeatureData.i32Data) ? true : false;
#endif
        }

        if (MEDIA_IS_SKU(skuTable, FtrPerCtxtPreemptionGranularityControl))
        {
            m_preemptionCntlRegisterOffset = MHW_RENDER_ENGINE_PREEMPTION_CONTROL_OFFSET;

            if (MEDIA_IS_SKU(skuTable, FtrMediaMidThreadLevelPreempt))
            {
                m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_MID_THREAD_PREEMPT_VALUE;
            }
            else if (MEDIA_IS_SKU(skuTable, FtrMediaThreadGroupLevelPreempt))
            {
                m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_THREAD_GROUP_PREEMPT_VALUE;
            }
            else if (MEDIA_IS_SKU(skuTable, FtrMediaMidBatchPreempt))
            {
                m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }

            // Set it to Mid Batch Pre-emption level (command level) to avoid render engine hang after preemption is turned on in ring buffer
            if (MEDIA_IS_WA(waTable, WaMidBatchPreemption))
            {
                m_preemptionCntlRegisterValue = MHW_RENDER_ENGINE_MID_BATCH_PREEMPT_VALUE;
            }
        }
    }

    MOS_STATUS EnablePreemption(PMOS_COMMAND_BUFFER cmdBuffer, MhwMiInterface* pMhwMiInterface) override
    {
        MOS_STATUS eStatus              = MOS_STATUS_SUCCESS;
        MEDIA_FEATURE_TABLE* skuTable   = nullptr;
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterParams = {};

        MHW_MI_CHK_NULL(skuTable);

        skuTable = this->m_osItf->pfnGetSkuTable(this->m_osItf);
        if (MEDIA_IS_SKU(skuTable, FtrPerCtxtPreemptionGranularityControl))
        {
            MOS_ZeroMemory(&loadRegisterParams, sizeof(loadRegisterParams));
            loadRegisterParams.dwRegister = m_preemptionCntlRegisterOffset;
            loadRegisterParams.dwData = m_preemptionCntlRegisterValue;
            MHW_MI_CHK_STATUS(pMhwMiInterface->AddMiLoadRegisterImmCmd(cmdBuffer, &loadRegisterParams));
        }

        return eStatus;
    }

    _MHW_SETCMD_OVERRIDE_DECL(PIPELINE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(PIPELINE_SELECT);
        cmd.DW0.PipelineSelection = (params.gpGpuPipe) ? cmd.PIPELINE_SELECTION_GPGPU : cmd.PIPELINE_SELECTION_MEDIA;
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(STATE_BASE_ADDRESS)
    {
        _MHW_SETCMD_CALLBASE(STATE_BASE_ADDRESS);

        MHW_RESOURCE_PARAMS resourceParams = {};

        resourceParams.dwLsbNum      = MHW_RENDER_ENGINE_STATE_BASE_ADDRESS_SHIFT;
        resourceParams.HwCommandType = MOS_STATE_BASE_ADDR;

        if (!Mos_ResourceIsNull(params.presGeneralState))
        {
            cmd.DW1_2.GeneralStateBaseAddressModifyEnable   = true;
            cmd.DW12.GeneralStateBufferSizeModifyEnable     = true;
            resourceParams.presResource                     = params.presGeneralState;
            resourceParams.dwOffset                         = 0;
            resourceParams.pdwCmd                           = cmd.DW1_2.Value;
            resourceParams.dwLocationInCmd                  = _MHW_CMD_DW_LOCATION(DW1_2.Value);

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW1_2.Value[0], 5, 10);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            if (params.mocs4GeneralState != 0)
            {
                cmd.DW1_2.GeneralStateMemoryObjectControlState = params.mocs4GeneralState;
            }
            cmd.DW12.GeneralStateBufferSize = (params.dwGeneralStateSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
        }

        if (this->m_osItf->bNoParsingAssistanceInKmd && !Mos_ResourceIsNull(&(this->m_currentCmdBuf->OsResource)))
        {
            uint32_t indirectStateOffset, indirectStateSize;
            MHW_MI_CHK_STATUS(this->m_osItf->pfnGetIndirectState(this->m_osItf, &indirectStateOffset, &indirectStateSize));

            // When KMD parsing assistance is not used,
            // UMD is required to set up the SSH
            // in the STATE_BASE_ADDRESS command.
            // All addresses used in the STATE_BASE_ADDRESS
            // command need to have the modify
            // bit associated with it set to 1.
            cmd.DW4_5.SurfaceStateBaseAddressModifyEnable  = true;
            resourceParams.presResource                    = &(this->m_currentCmdBuf->OsResource);
            resourceParams.dwOffset                        = indirectStateOffset;
            resourceParams.pdwCmd                          = cmd.DW4_5.Value;
            resourceParams.dwLocationInCmd                 = _MHW_CMD_DW_LOCATION(DW4_5.Value);

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW4_5.Value[0], 5, 10);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            if (params.mocs4SurfaceState != 0)
            {
                cmd.DW4_5.SurfaceStateMemoryObjectControlState = params.mocs4SurfaceState;
            }
        }

        if (!Mos_ResourceIsNull(params.presDynamicState))
        {
            cmd.DW6_7.DynamicStateBaseAddressModifyEnable  = true;
            cmd.DW13.DynamicStateBufferSizeModifyEnable    = true;
            resourceParams.presResource                     = params.presDynamicState;
            resourceParams.dwOffset                         = 0;
            resourceParams.pdwCmd                           = cmd.DW6_7.Value;
            resourceParams.dwLocationInCmd                  = _MHW_CMD_DW_LOCATION(DW6_7.Value);
            resourceParams.bIsWritable                      = params.bDynamicStateRenderTarget;

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW6_7.Value[0], 5, 10);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            if (params.mocs4DynamicState != 0)
            {
                cmd.DW6_7.DynamicStateMemoryObjectControlState = params.mocs4DynamicState;
            }

            cmd.DW13.DynamicStateBufferSize = (params.dwDynamicStateSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;

            //Reset bRenderTarget as it should be enabled only for Dynamic State
            resourceParams.bIsWritable = false;
        }

        if (!Mos_ResourceIsNull(params.presIndirectObjectBuffer))
        {
            cmd.DW8_9.IndirectObjectBaseAddressModifyEnable   = true;
            cmd.DW14.IndirectObjectBufferSizeModifyEnable     = true;
            resourceParams.presResource                       = params.presIndirectObjectBuffer;
            resourceParams.dwOffset                           = 0;
            resourceParams.pdwCmd                             = cmd.DW8_9.Value;
            resourceParams.dwLocationInCmd                    = _MHW_CMD_DW_LOCATION(DW8_9.Value);

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW8_9.Value[0], 5, 10);
            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            if (params.mocs4DynamicState != 0)
            {
                cmd.DW8_9.IndirectObjectMemoryObjectControlState = params.mocs4IndirectObjectBuffer;
            }
            cmd.DW14.IndirectObjectBufferSize = (params.dwIndirectObjectBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
        }

        if (!Mos_ResourceIsNull(params.presInstructionBuffer))
        {
            cmd.DW10_11.InstructionBaseAddressModifyEnable   = true;
            cmd.DW15.InstructionBufferSizeModifyEnable       = true;
            resourceParams.presResource                      = params.presInstructionBuffer;
            resourceParams.dwOffset                          = 0;
            resourceParams.pdwCmd                            = cmd.DW10_11.Value;
            resourceParams.dwLocationInCmd                   = _MHW_CMD_DW_LOCATION(DW10_11.Value);

            // upper bound of the allocated resource will not be set
            resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

            InitMocsParams(resourceParams, &cmd.DW10_11.Value[0], 5, 10);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            if (params.mocs4InstructionCache != 0)
            {
                cmd.DW10_11.InstructionMemoryObjectControlState = params.mocs4InstructionCache;
            }
            cmd.DW15.InstructionBufferSize = (params.dwInstructionBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
        }

        // stateless dataport access
        cmd.DW3.StatelessDataPortAccessMemoryObjectControlState = params.mocs4StatelessDataport;
        cmd.DW3.L1CachePolicy                                   = params.l1CacheConfig;

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

    _MHW_SETCMD_OVERRIDE_DECL(PALETTE_ENTRY)
    {
        _MHW_SETCMD_CALLBASE(PALETTE_ENTRY);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(STATE_SIP)
    {
        _MHW_SETCMD_CALLBASE(STATE_SIP);
        cmd.DW1_2.SystemInstructionPointer = (uint64_t)(params.dwSipBase >> 4);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(GPGPU_CSR_BASE_ADDRESS)
    {
        _MHW_SETCMD_CALLBASE(GPGPU_CSR_BASE_ADDRESS);
        MHW_RESOURCE_PARAMS resourceParams = {};

        if (!Mos_ResourceIsNull(&(this->m_currentCmdBuf->OsResource)))
        {
            resourceParams.presResource    = &(this->m_currentCmdBuf->OsResource);
            resourceParams.pdwCmd          = cmd.DW1_2.Value;
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW1_2.Value);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(_3DSTATE_BINDING_TABLE_POOL_ALLOC)
    {
        _MHW_SETCMD_CALLBASE(_3DSTATE_BINDING_TABLE_POOL_ALLOC);

        uint32_t indirect_state_offset = 0, indirect_state_size = 0;
        MHW_MI_CHK_STATUS(this->m_osItf->pfnGetIndirectState(this->m_osItf,
            &indirect_state_offset,
            &indirect_state_size));

        MHW_RESOURCE_PARAMS resource_params = {};
        if (!Mos_ResourceIsNull(&(this->m_currentCmdBuf->OsResource)))
        {
            InitMocsParams(resource_params, &cmd.DW1_2.Value[0], 1, 6);
            resource_params.dwLsbNum        = MHW_RENDER_ENGINE_STATE_BASE_ADDRESS_SHIFT;
            resource_params.HwCommandType   = MOS_STATE_BASE_ADDR;
            resource_params.presResource    = &this->m_currentCmdBuf->OsResource;
            resource_params.dwOffset        = indirect_state_offset;
            resource_params.pdwCmd          = cmd.DW1_2.Value;
            resource_params.dwLocationInCmd = _MHW_CMD_DW_LOCATION(DW1_2.Value);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resource_params));
         }

        cmd.DW3.BindingTablePoolBufferSize = indirect_state_size;

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(CFE_STATE)
    {
        _MHW_SETCMD_CALLBASE(CFE_STATE);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(COMPUTE_WALKER)
    {
        _MHW_SETCMD_CALLBASE(COMPUTE_WALKER);

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(STATE_COMPUTE_MODE)
    {
        _MHW_SETCMD_CALLBASE(STATE_COMPUTE_MODE);

        return MOS_STATUS_SUCCESS;
    }
};
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_IMPL_H__
