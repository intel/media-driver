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
//! \file     mhw_vdbox_huc_impl.h
//! \brief    MHW VDBOX HUC interface common base
//! \details
//!

#ifndef __MHW_VDBOX_HUC_IMPL_H__
#define __MHW_VDBOX_HUC_IMPL_H__

#include "mhw_vdbox_huc_itf.h"
#include "mhw_impl.h"

namespace mhw
{
namespace vdbox
{
namespace huc
{
static constexpr uint32_t MEMORY_ADDRESS_ATTRIBUTES_MOCS_CLEAN_MASK = 0xFFFFFF81;

static constexpr uint32_t HUC_UKERNEL_HDR_INFO_REG_OFFSET_NODE_1_INIT = 0x1C2014;
static constexpr uint32_t HUC_STATUS_REG_OFFSET_NODE_1_INIT           = 0x1C2000;
static constexpr uint32_t HUC_STATUS2_REG_OFFSET_NODE_1_INIT          = 0x1C23B0;
static constexpr uint32_t HUC_LOAD_INFO_REG_OFFSET_NODE_1_INIT        = 0xC1DC;

template <typename cmd_t>
class Impl : public Itf, public mhw::Impl
{
    _HUC_CMD_DEF(_MHW_CMD_ALL_DEF_FOR_IMPL);

public:
    MOS_STATUS SetCacheabilitySettings(MHW_MEMORY_OBJECT_CONTROL_PARAMS settings[MOS_CODEC_RESOURCE_USAGE_END_CODEC]) override
    {
        MHW_FUNCTION_ENTER;

        MHW_CHK_NULL_RETURN(settings);

        size_t size = MOS_CODEC_RESOURCE_USAGE_END_CODEC * sizeof(MHW_MEMORY_OBJECT_CONTROL_PARAMS);

        return MOS_SecureMemcpy(m_cacheabilitySettings, size, settings, size);
    }

    //!
    //! \brief    Get mmio registers
    //!
    //! \param    [in] index
    //!           mmio registers index.
    //!
    //! \return   [out] MmioRegistersHuc*
    //!           mmio registers got.
    //!
    HucMmioRegisters* GetMmioRegisters(MHW_VDBOX_NODE_IND index) override
    {
        if (index < MHW_VDBOX_NODE_MAX)
        {
            return &m_mmioRegisters[index];
        }
        else
        {
            MHW_ASSERT("index is out of range!");
            return &m_mmioRegisters[MHW_VDBOX_NODE_1];
        }
    }

private:
    void InitMmioRegisters()
    {
        HucMmioRegisters *mmioRegisters = &m_mmioRegisters[MHW_VDBOX_NODE_1];

        mmioRegisters->hucUKernelHdrInfoRegOffset = HUC_UKERNEL_HDR_INFO_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->hucStatusRegOffset         = HUC_STATUS_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->hucStatus2RegOffset        = HUC_STATUS2_REG_OFFSET_NODE_1_INIT;
        mmioRegisters->hucLoadInfoOffset          = HUC_LOAD_INFO_REG_OFFSET_NODE_1_INIT;

        m_mmioRegisters[MHW_VDBOX_NODE_2] = m_mmioRegisters[MHW_VDBOX_NODE_1];
    }

    uint32_t GetHucStatusReEncodeMask() override
    {
        return m_hucStatusReEncodeMask;
    }

    uint32_t GetHucStatusHevcS2lFailureMask() override
    {
        return m_hucStatusHevcS2lFailureMask;
    }

    uint32_t GetHucStatus2ImemLoadedMask() override
    {
        return m_hucStatus2ImemLoadedMask;
    }

    uint32_t GetHucErrorFlagsMask() override
    {
        return m_hucErrorFlagsMask;
    }

    uint32_t GetHucProductFamily() override
    {
        return m_hucFamily;
    }

protected:
    using base_t = Itf;
    HucMmioRegisters      m_mmioRegisters[MHW_VDBOX_NODE_MAX] = {};  //!< HuC mmio registers
    MhwCpInterface        *m_cpItf = nullptr;
    static const uint32_t m_hucStatusReEncodeMask             = 0x80000000;
    static const uint32_t m_hucStatusHevcS2lFailureMask       = 0x8000;
    static const uint32_t m_hucStatus2ImemLoadedMask          = 0x40;
    static const uint32_t m_hucErrorFlagsMask                 = 0xFFFE;          //!< HuC error 2 flags mask
    static const uint32_t m_hucFamily = 8;

    MHW_MEMORY_OBJECT_CONTROL_PARAMS m_cacheabilitySettings[MOS_CODEC_RESOURCE_USAGE_END_CODEC] = {};

    Impl(PMOS_INTERFACE osItf, MhwCpInterface *cpItf) : mhw::Impl(osItf)
    {
        m_cpItf = cpItf;

        InitMmioRegisters();
    }

    _MHW_SETCMD_OVERRIDE_DECL(HUC_PIPE_MODE_SELECT)
    {
        _MHW_SETCMD_CALLBASE(HUC_PIPE_MODE_SELECT);

        if (!params.disableProtectionSetting)
        {
            MHW_MI_CHK_STATUS(m_cpItf->SetProtectionSettingsForHucPipeModeSelect((uint32_t *)&cmd));
        }

#define DO_FIELDS()                                                          \
    DO_FIELD(DW1, IndirectStreamOutEnable, params.streamOutEnabled ? 1 : 0); \
    DO_FIELD(DW2, MediaSoftResetCounterPer1000Clocks, params.mediaSoftResetCounterValue);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HUC_IND_OBJ_BASE_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(HUC_IND_OBJ_BASE_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        resourceParams.dwLsbNum                          = MHW_VDBOX_HUC_UPPER_BOUND_STATE_SHIFT;
        resourceParams.dwUpperBoundLocationOffsetFromCmd = 3;
        resourceParams.HwCommandType                     = MOS_HUC_IND_OBJ_BASE_ADDR;

        if (!Mos_ResourceIsNull(params.DataBuffer))
        {
            resourceParams.presResource    = params.DataBuffer;
            resourceParams.dwOffset        = params.DataOffset;
            resourceParams.pdwCmd          = cmd.HucIndirectStreamInObjectbaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd = 1;
            resourceParams.bIsWritable     = false;
            resourceParams.dwSize          = params.DataSize;

            InitMocsParams(resourceParams, &cmd.HucIndirectStreamInObjectbaseAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        if (!Mos_ResourceIsNull(params.StreamOutObjectBuffer))
        {
            resourceParams.presResource    = params.StreamOutObjectBuffer;
            resourceParams.dwOffset        = params.StreamOutObjectOffset;
            resourceParams.pdwCmd          = cmd.HucIndirectStreamOutObjectbaseAddress.DW0_1.Value;
            resourceParams.dwLocationInCmd = 6;
            resourceParams.bIsWritable     = true;
            resourceParams.dwSize          = params.StreamOutObjectSize;

            InitMocsParams(resourceParams, &cmd.HucIndirectStreamOutObjectbaseAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));
        }

        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HUC_STREAM_OBJECT)
    {
        _MHW_SETCMD_CALLBASE(HUC_STREAM_OBJECT);
#define DO_FIELDS()                                                                       \
    DO_FIELD(DW1, IndirectStreamInDataLength, params.IndirectStreamInDataLength);         \
    DO_FIELD(DW2, IndirectStreamInStartAddress, params.IndirectStreamInStartAddress);     \
    DO_FIELD(DW2, HucProcessing, params.HucProcessing);                                   \
    DO_FIELD(DW3, IndirectStreamOutStartAddress, params.IndirectStreamOutStartAddress);   \
    DO_FIELD(DW4, HucBitstreamEnable, params.HucBitstreamEnable);                         \
    DO_FIELD(DW4, StreamOut, params.StreamOut);                                           \
    DO_FIELD(DW4, EmulationPreventionByteRemoval, params.EmulationPreventionByteRemoval); \
    DO_FIELD(DW4, StartCodeSearchEngine, params.StartCodeSearchEngine);                   \
    DO_FIELD(DW4, Drmlengthmode, params.Drmlengthmode);                                   \
    DO_FIELD(DW4, StartCodeByte2, params.StartCodeByte2);                                 \
    DO_FIELD(DW4, StartCodeByte1, params.StartCodeByte1);                                 \
    DO_FIELD(DW4, StartCodeByte0, params.StartCodeByte0);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HUC_IMEM_STATE)
    {
        _MHW_SETCMD_CALLBASE(HUC_IMEM_STATE);

#define DO_FIELDS() \
    DO_FIELD(DW4, HucFirmwareDescriptor, params.kernelDescriptor);

#include "mhw_hwcmd_process_cmdfields.h"
    }

    _MHW_SETCMD_OVERRIDE_DECL(HUC_DMEM_STATE)
    {
        _MHW_SETCMD_CALLBASE(HUC_DMEM_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum      = MHW_VDBOX_HUC_GENERAL_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_HUC_DMEM;

        if (!Mos_ResourceIsNull(params.hucDataSource))
        {
            resourceParams.presResource    = params.hucDataSource;
            resourceParams.dwOffset        = 0;
            resourceParams.pdwCmd          = (cmd.HucDataSourceBaseAddress.DW0_1.Value);
            resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(HucDataSourceBaseAddress);
            resourceParams.bIsWritable     = false;

            InitMocsParams(resourceParams, &cmd.HucDataSourceAttributes.DW0.Value, 1, 6);

            MHW_MI_CHK_STATUS(AddResourceToCmd(
                this->m_osItf,
                this->m_currentCmdBuf,
                &resourceParams));

            // set HuC data destination address
            cmd.DW4.HucDataDestinationBaseAddress = params.dmemOffset >> MHW_VDBOX_HUC_GENERAL_STATE_SHIFT;

            // set data length
            cmd.DW5.HucDataLength = params.dataLength >> MHW_VDBOX_HUC_GENERAL_STATE_SHIFT;
        }
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HUC_VIRTUAL_ADDR_STATE)
    {
        _MHW_SETCMD_CALLBASE(HUC_VIRTUAL_ADDR_STATE);

        MHW_RESOURCE_PARAMS resourceParams = {};

        // set up surface 0~15
        MOS_ZeroMemory(&resourceParams, sizeof(resourceParams));
        resourceParams.dwLsbNum      = MHW_VDBOX_HUC_UPPER_BOUND_STATE_SHIFT;
        resourceParams.HwCommandType = MOS_HUC_VIRTUAL_ADDR;

        for (int i = 0; i < 16; i++)
        {
            if (params.regionParams[i].presRegion)
            {
                resourceParams.presResource    = params.regionParams[i].presRegion;
                resourceParams.dwOffset        = params.regionParams[i].dwOffset;
                resourceParams.bIsWritable     = params.regionParams[i].isWritable;
                resourceParams.pdwCmd          = cmd.HucVirtualAddressRegion[i].HucSurfaceBaseAddressVirtualaddrregion015.DW0_1.Value;
                resourceParams.dwLocationInCmd = _MHW_CMD_DW_LOCATION(HucVirtualAddressRegion[i].HucSurfaceBaseAddressVirtualaddrregion015);

                InitMocsParams(resourceParams, &cmd.HucVirtualAddressRegion[i].HucSurfaceVirtualaddrregion015.DW0.Value, 1, 6);

                MHW_MI_CHK_STATUS(AddResourceToCmd(
                    this->m_osItf,
                    this->m_currentCmdBuf,
                    &resourceParams));
            }
        }
        return MOS_STATUS_SUCCESS;
    }

    _MHW_SETCMD_OVERRIDE_DECL(HUC_START)
    {
        _MHW_SETCMD_CALLBASE(HUC_START);

#define DO_FIELDS() \
    DO_FIELD(DW1, Laststreamobject, params.lastStreamObject ? 1 : 0);

#include "mhw_hwcmd_process_cmdfields.h"
    }
MEDIA_CLASS_DEFINE_END(mhw__vdbox__huc__Impl)
};
}  // namespace huc
}  // namespace vdbox
}  // namespace mhw

#endif  // __MHW_VDBOX_HUC_IMPL_H__
