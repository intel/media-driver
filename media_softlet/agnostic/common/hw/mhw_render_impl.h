/*
* Copyright (c) 2021-2024, Intel Corporation
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

    MOS_STATUS SetL3Cache(PMOS_COMMAND_BUFFER cmdBuffer, std::shared_ptr<mhw::mi::Itf> miItf) override
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

    MHW_RENDER_ENGINE_L3_CACHE_CONFIG* GetL3CacheConfig() override
    { 
        return &m_l3CacheConfig;
    }

    MOS_STATUS SetupInlineData() override
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    using base_t = Itf;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS    m_cacheabilitySettings[MOS_HW_RESOURCE_DEF_MAX] = {};
    MHW_MI_MMIOREGISTERS                m_mmioRegisters = {};
    MHW_RENDER_ENGINE_L3_CACHE_CONFIG   m_l3CacheConfig = {};
    
    bool        m_preemptionEnabled = false;
    uint32_t    m_preemptionCntlRegisterOffset = 0;
    uint32_t    m_preemptionCntlRegisterValue = 0;
    MHW_STATE_HEAP_INTERFACE *m_stateHeapInterface = nullptr;
    MHW_RENDER_ENGINE_CAPS    m_hwCaps             = {};
    uint8_t                   m_heapMode           = MHW_RENDER_HAL_MODE;

    Impl(PMOS_INTERFACE osItf) : mhw::Impl(osItf)
    {
        MHW_FUNCTION_ENTER;

        if (!osItf->bUsesGfxAddress && !osItf->bUsesPatchList)
        {
            MHW_ASSERTMESSAGE("No valid addressing mode indicated");
            return;
        }

        m_stateHeapInterface = nullptr;

        MEDIA_SYSTEM_INFO *gtSystemInfo     = osItf->pfnGetGtSystemInfo(osItf);

        InitPlatformCaps(gtSystemInfo);

        InitPreemption();

        if (Mhw_StateHeapInterface_InitInterface(
                &m_stateHeapInterface,
                osItf,
                m_heapMode) != MOS_STATUS_SUCCESS)
        {
            MHW_ASSERTMESSAGE("State heap initialization failed!");
            return;
        }
    }

    ~Impl()
    {
        MHW_FUNCTION_ENTER;

        if (m_stateHeapInterface)
        {
            m_stateHeapInterface->pfnDestroy(m_stateHeapInterface);
        }
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

    //!
    //! \brief    Allocates the MHW render interface internal parameters
    //! \details  Internal MHW function to allocate all parameters needed for the
    //!           render interface including the state heap interface
    //! \param    MHW_STATE_HEAP_SETTINGS stateHeapSettings
    //!           [in] Setting used to initialize the state heap interface
    //! \return   MOS_STATUS
    //!           MOS_STATUS_SUCCESS if success, else fail reason
    //!
    MOS_STATUS AllocateHeaps(
        MHW_STATE_HEAP_SETTINGS         stateHeapSettings) override
    {
        MOS_STATUS                      eStatus = MOS_STATUS_SUCCESS;
        MHW_STATE_HEAP_INTERFACE        *stateHeapInterface = nullptr;

        MHW_FUNCTION_ENTER;

        if ((stateHeapSettings.dwIshSize > 0 ||
            stateHeapSettings.dwDshSize > 0 ) &&
            stateHeapSettings.dwNumSyncTags > 0)
        {
            MHW_MI_CHK_STATUS(m_stateHeapInterface->pfnCreate(
                &m_stateHeapInterface,
                stateHeapSettings));
        }

        return eStatus;
    }

    PMHW_STATE_HEAP_INTERFACE GetStateHeapInterface() override
    {
        MHW_FUNCTION_ENTER;

        return m_stateHeapInterface;
    }

    void InitPlatformCaps(
        MEDIA_SYSTEM_INFO         *gtSystemInfo)
    {
        if (gtSystemInfo == nullptr)
        {
            MHW_ASSERTMESSAGE("Invalid input pointer provided");
            return;
        }

        MOS_ZeroMemory(&m_hwCaps, sizeof(MHW_RENDER_ENGINE_CAPS));

        m_hwCaps.dwMaxUnormSamplers       = MHW_RENDER_ENGINE_SAMPLERS_MAX;
        m_hwCaps.dwMaxAVSSamplers         = MHW_RENDER_ENGINE_SAMPLERS_AVS_MAX;
        m_hwCaps.dwMaxBTIndex             = MHW_RENDER_ENGINE_SSH_SURFACES_PER_BT_MAX - 1;
        m_hwCaps.dwMaxThreads             = gtSystemInfo->ThreadCount;
        m_hwCaps.dwMaxMediaPayloadSize    = MHW_RENDER_ENGINE_MEDIA_PALOAD_SIZE_MAX;
        m_hwCaps.dwMaxURBSize             = MHW_RENDER_ENGINE_URB_SIZE_MAX;
        m_hwCaps.dwMaxURBEntries          = MHW_RENDER_ENGINE_URB_ENTRIES_MAX;
        m_hwCaps.dwMaxSubslice            = gtSystemInfo->MaxSubSlicesSupported;
        m_hwCaps.dwMaxEUIndex             = MHW_RENDER_ENGINE_EU_INDEX_MAX;
        m_hwCaps.dwNumThreadsPerEU        = (gtSystemInfo->EUCount > 0) ?
            gtSystemInfo->ThreadCount / gtSystemInfo->EUCount : 0;
        m_hwCaps.dwSizeRegistersPerThread = MHW_RENDER_ENGINE_SIZE_REGISTERS_PER_THREAD;

        m_hwCaps.dwMaxInterfaceDescriptorEntries  = MHW_RENDER_ENGINE_INTERFACE_DESCRIPTOR_ENTRIES_MAX;
        m_hwCaps.dwMaxURBEntryAllocationSize      =
            m_hwCaps.dwMaxCURBEAllocationSize     =
            m_hwCaps.dwMaxURBSize - m_hwCaps.dwMaxInterfaceDescriptorEntries;
    }

    void InitPreemption()
    {
        MHW_FUNCTION_ENTER;
        MHW_CHK_NULL_NO_STATUS_RETURN(this->m_osItf);

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
        if (this->m_userSettingPtr != nullptr)
        {
            ReadUserSettingForDebug(
                this->m_userSettingPtr,
                m_preemptionEnabled,
                __MEDIA_USER_FEATURE_VALUE_MEDIA_PREEMPTION_ENABLE,
                MediaUserSetting::Group::Device);
        }
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

            MHW_NORMALMESSAGE("InitPreemption m_preemptionCntlRegisterValue = %x", m_preemptionCntlRegisterValue);
        }
    }

    MOS_STATUS EnablePreemption(PMOS_COMMAND_BUFFER cmdBuffer, std::shared_ptr<mhw::mi::Itf> miItf) override
    {
        MHW_FUNCTION_ENTER;
        MOS_STATUS eStatus              = MOS_STATUS_SUCCESS;
        MEDIA_FEATURE_TABLE* skuTable   = nullptr;
        MHW_MI_LOAD_REGISTER_IMM_PARAMS loadRegisterParams = {};

        MHW_MI_CHK_NULL(cmdBuffer);
        MHW_MI_CHK_NULL(miItf);
        MHW_MI_CHK_NULL(this->m_osItf);

        skuTable = this->m_osItf->pfnGetSkuTable(this->m_osItf);
        MHW_MI_CHK_NULL(skuTable);
        if (MEDIA_IS_SKU(skuTable, FtrPerCtxtPreemptionGranularityControl))
        {
            auto& par = miItf->MHW_GETPAR_F(MI_LOAD_REGISTER_IMM)();
            par = {};
            par.dwRegister = m_preemptionCntlRegisterOffset;
            par.dwData     = m_preemptionCntlRegisterValue;
            MHW_MI_CHK_STATUS(miItf->MHW_ADDCMD_F(MI_LOAD_REGISTER_IMM)(cmdBuffer));

            MHW_NORMALMESSAGE("EnablePreemption Set %x to reg %x", m_preemptionCntlRegisterValue, m_preemptionCntlRegisterOffset);
        }

        return eStatus;
    }

    bool IsPreemptionEnabled() override
    {
        return m_preemptionEnabled;
    }

    void GetSamplerResolutionAlignUnit(bool isAVSSampler, uint32_t &widthAlignUnit, uint32_t &heightAlignUnit) override
    {
        // enable 2 plane NV12 when width is not multiple of 2 or height is
        // not multiple of 4. For AVS sampler, no limitation for 4 alignment.
        widthAlignUnit  = isAVSSampler ? MHW_AVS_SAMPLER_WIDTH_ALIGN_UNIT : MHW_SAMPLER_WIDTH_ALIGN_UNIT;
        heightAlignUnit = isAVSSampler ? MHW_AVS_SAMPLER_HEIGHT_ALIGN_UNIT : MHW_SAMPLER_HEIGHT_ALIGN_UNIT;
    }

    MHW_RENDER_ENGINE_CAPS* GetHwCaps() override
    {
        return &m_hwCaps;
    }

    _MHW_SETCMD_OVERRIDE_DECL(PIPELINE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(PIPELINE_SELECT);
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(STATE_BASE_ADDRESS)
    {
        _MHW_SETCMD_CALLBASE(STATE_BASE_ADDRESS);

        if (params.addressDis == false)
        {
            MHW_RESOURCE_PARAMS resourceParams = {};

            resourceParams.dwLsbNum      = MHW_RENDER_ENGINE_STATE_BASE_ADDRESS_SHIFT;
            resourceParams.HwCommandType = MOS_STATE_BASE_ADDR;

            if (!Mos_ResourceIsNull(params.presGeneralState))
            {
                cmd.DW1_2.GeneralStateBaseAddressModifyEnable = true;
                cmd.DW12.GeneralStateBufferSizeModifyEnable   = true;
                resourceParams.presResource                   = params.presGeneralState;
                resourceParams.dwOffset                       = 0;
                resourceParams.pdwCmd                         = cmd.DW1_2.Value;
                resourceParams.dwLocationInCmd                = _MHW_CMD_DW_LOCATION(DW1_2.Value);

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
                MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW1_2.GeneralStateMemoryObjectControlState);
                MHW_NORMALMESSAGE(
                    "Feature Graph: Cache settings of DW1_2 GeneralState in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                    cmd.DW1_2.GeneralStateMemoryObjectControlState,
                    (cmd.DW1_2.GeneralStateMemoryObjectControlState >> 1) & 0x0000003f);
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
                cmd.DW4_5.SurfaceStateBaseAddressModifyEnable = true;
                resourceParams.presResource                   = &(this->m_currentCmdBuf->OsResource);
                resourceParams.dwOffset                       = indirectStateOffset;
                resourceParams.pdwCmd                         = cmd.DW4_5.Value;
                resourceParams.dwLocationInCmd                = _MHW_CMD_DW_LOCATION(DW4_5.Value);

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
                MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW4_5.SurfaceStateMemoryObjectControlState);
                MHW_NORMALMESSAGE(
                    "Feature Graph: Cache settings of DW4_5 SurfaceState in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                    cmd.DW4_5.SurfaceStateMemoryObjectControlState,
                    (cmd.DW4_5.SurfaceStateMemoryObjectControlState >> 1) & 0x0000003f);
            }

            if (!Mos_ResourceIsNull(params.presDynamicState))
            {
                cmd.DW6_7.DynamicStateBaseAddressModifyEnable = true;
                cmd.DW13.DynamicStateBufferSizeModifyEnable   = true;
                resourceParams.presResource                   = params.presDynamicState;
                resourceParams.dwOffset                       = 0;
                resourceParams.pdwCmd                         = cmd.DW6_7.Value;
                resourceParams.dwLocationInCmd                = _MHW_CMD_DW_LOCATION(DW6_7.Value);
                resourceParams.bIsWritable                    = params.bDynamicStateRenderTarget;

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

                MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW6_7.DynamicStateMemoryObjectControlState);
                MHW_NORMALMESSAGE(
                    "Feature Graph: Cache settings of DW6_7 DynamicState in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                    cmd.DW6_7.DynamicStateMemoryObjectControlState,
                    (cmd.DW6_7.DynamicStateMemoryObjectControlState >> 1) & 0x0000003f);
                cmd.DW13.DynamicStateBufferSize = (params.dwDynamicStateSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;

                //Reset isOutput as it should be enabled only for Dynamic State
                resourceParams.bIsWritable = false;
            }

            if (!Mos_ResourceIsNull(params.presIndirectObjectBuffer))
            {
                cmd.DW8_9.IndirectObjectBaseAddressModifyEnable = true;
                cmd.DW14.IndirectObjectBufferSizeModifyEnable   = true;
                resourceParams.presResource                     = params.presIndirectObjectBuffer;
                resourceParams.dwOffset                         = 0;
                resourceParams.pdwCmd                           = cmd.DW8_9.Value;
                resourceParams.dwLocationInCmd                  = _MHW_CMD_DW_LOCATION(DW8_9.Value);

                // upper bound of the allocated resource will not be set
                resourceParams.dwUpperBoundLocationOffsetFromCmd = 0;

                InitMocsParams(resourceParams, &cmd.DW8_9.Value[0], 5, 10);
                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));

                if (params.mocs4IndirectObjectBuffer != 0)
                {
                    cmd.DW8_9.IndirectObjectMemoryObjectControlState = params.mocs4IndirectObjectBuffer;
                }
                MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW8_9.IndirectObjectMemoryObjectControlState);
                MHW_NORMALMESSAGE(
                    "Feature Graph: Cache settings of DW8_9 IndirectObject in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                    cmd.DW8_9.IndirectObjectMemoryObjectControlState,
                    (cmd.DW8_9.IndirectObjectMemoryObjectControlState >> 1) & 0x0000003f);
                cmd.DW14.IndirectObjectBufferSize = (params.dwIndirectObjectBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
            }

            if (!Mos_ResourceIsNull(params.presInstructionBuffer))
            {
                cmd.DW10_11.InstructionBaseAddressModifyEnable = true;
                cmd.DW15.InstructionBufferSizeModifyEnable     = true;
                resourceParams.presResource                    = params.presInstructionBuffer;
                resourceParams.dwOffset                        = 0;
                resourceParams.pdwCmd                          = cmd.DW10_11.Value;
                resourceParams.dwLocationInCmd                 = _MHW_CMD_DW_LOCATION(DW10_11.Value);

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
                MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW10_11.InstructionMemoryObjectControlState);
                MHW_NORMALMESSAGE(
                    "Feature Graph: Cache settings of DW10_11 Instruction in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                    cmd.DW10_11.InstructionMemoryObjectControlState,
                    (cmd.DW10_11.InstructionMemoryObjectControlState >> 1) & 0x0000003f);
                cmd.DW15.InstructionBufferSize = (params.dwInstructionBufferSize + MHW_PAGE_SIZE - 1) / MHW_PAGE_SIZE;
            }

            // stateless dataport access
            cmd.DW3.StatelessDataPortAccessMemoryObjectControlState = params.mocs4StatelessDataport;
            MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW3.StatelessDataPortAccessMemoryObjectControlState);
            MHW_NORMALMESSAGE(
                "Feature Graph: Cache settings of DW3 StatelessDataPortAccess in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                cmd.DW3.StatelessDataPortAccessMemoryObjectControlState,
                (cmd.DW3.StatelessDataPortAccessMemoryObjectControlState >> 1) & 0x0000003f);

            MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW16_17.BindlessSurfaceStateMemoryObjectControlState);
            MHW_NORMALMESSAGE(
                "Feature Graph: Cache settings of DW16_17 BindlessSurfaceState in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                cmd.DW16_17.BindlessSurfaceStateMemoryObjectControlState,
                (cmd.DW16_17.BindlessSurfaceStateMemoryObjectControlState >> 1) & 0x0000003f);

            MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"STATE_BASE_ADDRESS"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW19_20.BindlessSamplerStateMemoryObjectControlState);
            MHW_NORMALMESSAGE(
                "Feature Graph: Cache settings of DW19_20 BindlessSamplerState in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                cmd.DW19_20.BindlessSamplerStateMemoryObjectControlState,
                (cmd.DW19_20.BindlessSamplerStateMemoryObjectControlState >> 1) & 0x0000003f);
        }

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
            if (params.mocs4SurfaceState != 0)
            {
                cmd.DW1_2.SurfaceObjectControlState = params.mocs4SurfaceState;
            }

            MT_LOG2(MT_VP_MHW_CACHE_MOCS_TABLE, MT_NORMAL, MT_VP_MHW_CACHE_MEMORY_OBJECT_NAME, *((int64_t *)"3DSTATE_BINDING_TABLE_POOL_ALLOC"), MT_VP_MHW_CACHE_MEMORY_OBJECT_CONTROL_STATE, (int64_t)cmd.DW1_2.SurfaceObjectControlState);
            MHW_NORMALMESSAGE(
                "Feature Graph: Cache settings of StatelessDataPortAccess in STATE_BASE_ADDRESS: SurfaceMemoryObjectControlState %u, Index to Mocs table %u",
                cmd.DW1_2.SurfaceObjectControlState,
                (cmd.DW1_2.SurfaceObjectControlState >> 1) & 0x0000003f);
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
MEDIA_CLASS_DEFINE_END(mhw__render__Impl)
};
}  // namespace render
}  // namespace mhw

#endif  // __MHW_RENDER_IMPL_H__
