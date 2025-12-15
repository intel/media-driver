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
//! \file     decode_vvc_s2l_packet_xe3p_lpm_base.cpp
//! \brief    Defines the implementation of VVC decode S2L packet
//!
#include "decode_vvc_s2l_packet_xe3p_lpm_base.h"
#include "codec_hw_xe3p_lpm_base.h"
#include "media_interfaces_huc_kernel_source.h"
#include "decode_resource_auto_lock.h"
#include "mos_os_cp_interface_specific.h"
#include "decode_vvc_pipeline_xe3p_lpm_base.h"
#include "decode_vvc_slice_packet_xe3p_lpm_base.h"

namespace decode
{
    MOS_STATUS VvcDecodeS2LPktXe3P_Lpm_Base::Init()
    {
        DECODE_FUNC_CALL();

        DECODE_CHK_NULL(m_vvcPipeline);
        m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
        DECODE_CHK_NULL(m_hucKernelSource);
        m_isPpgttMode = m_hucKernelSource->IsPpgttMode(m_vvcPipeline->GetSkuTable(), m_vvcPipeline->GetUserSetting());

        DECODE_CHK_STATUS(VvcDecodeS2LPkt::Init());

        if (m_isPpgttMode)
        {
            std::shared_ptr<mhw::vdbox::huc::ItfExt> itf =
                std::dynamic_pointer_cast<mhw::vdbox::huc::ItfExt>(m_hucItf);
            DECODE_CHK_COND(!itf, "cannot convert huc itf to Huc ext itf!");
            m_pictureStatesSize += itf->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
            m_picturePatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfExt::HUC_IMEM_ADDR_CMD);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPktXe3P_Lpm_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded)
    {
        DECODE_FUNC_CALL();
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        if (prologNeeded)
        {
            DECODE_CHK_STATUS(AddForceWakeup(cmdBuffer, false, true));
            DECODE_CHK_STATUS(SendPrologCmds(cmdBuffer));
        }

        DECODE_CHK_STATUS(PackPictureLevelCmds(cmdBuffer));
        DECODE_CHK_STATUS(PackSliceLevelCmds(cmdBuffer));
        DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
        // Flush the engine to ensure memory written out
        DECODE_CHK_STATUS(MemoryFlush(cmdBuffer));

        MOS_RESOURCE *osResource = nullptr;
        uint32_t      offset     = 0;

        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Mask, osResource, offset));

        DECODE_CHK_STATUS(StoreHucStatusRegister(cmdBuffer));

        // Check HuC_STATUS2 bit6, if bit6 > 0 HW continue execution following cmd, otherwise it send a COND BB END cmd.
        uint32_t compareOperation = mhw::mi::COMPARE_OPERATION_MADGREATERTHANIDD;
        CodechalHwInterfaceXe3P_Lpm_Base *hwInterface = dynamic_cast<CodechalHwInterfaceXe3P_Lpm_Base *>(m_hwInterface);
        DECODE_CHK_NULL(hwInterface);
        DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(
            osResource, offset, 0, false, false, compareOperation, &cmdBuffer));

        if (!m_vvcPipeline->IsSingleTaskPhaseSupported())
        {
            DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPktXe3P_Lpm_Base::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        if (m_isPpgttMode)
        {
            std::shared_ptr<mhw::vdbox::huc::ItfExt> itf =
                std::dynamic_pointer_cast<mhw::vdbox::huc::ItfExt>(m_hucItf);
            DECODE_CHK_COND(!itf, "cannot convert huc itf to Huc ext itf!");

            SETPAR_AND_ADDCMD(HUC_IMEM_ADDR, itf, &cmdBuffer);
        }

        DECODE_CHK_STATUS(AddCmd_HUC_IMEM_STATE(cmdBuffer));
        DECODE_CHK_STATUS(AddCmd_HUC_PIPE_MODE_SELECT(cmdBuffer));
        SETPAR_AND_ADDCMD(HUC_IND_OBJ_BASE_ADDR_STATE, m_hucItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, &cmdBuffer);
        SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, &cmdBuffer);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPktXe3P_Lpm_Base::AllocateResources()
    {
        DECODE_CHK_STATUS(VvcDecodeS2LPkt::AllocateResources());

        if (m_isPpgttMode && m_kernelBinBuffer == nullptr)
        {
            HucKernelSource::HucBinary hucBinary;
            DECODE_CHK_STATUS(m_hucKernelSource->GetKernelBin(HucKernelSource::vvcS2lKernelId, hucBinary));
            DECODE_CHK_NULL(hucBinary.m_data);

            m_kernelBinBuffer = m_allocator->AllocateBuffer(
                MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE), "VVCS2LkernelBinBuffer",
                resourceInternalReadWriteCache, lockableVideoMem);
            DECODE_CHK_NULL(m_kernelBinBuffer);

            ResourceAutoLock kernelBinResLock(m_allocator, &m_kernelBinBuffer->OsResource);
            void *kernelBinBuffer = kernelBinResLock.LockResourceForWrite();
            DECODE_CHK_NULL(kernelBinBuffer);

            MOS_SecureMemcpy(kernelBinBuffer, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPktXe3P_Lpm_Base::Destroy()
    {
        if (m_hucKernelSource != nullptr)
        {
            DECODE_CHK_STATUS(m_hucKernelSource->ReportMode(m_vvcPipeline->GetSkuTable(), m_vvcPipeline->GetUserSetting()));
        }
        if (m_allocator != nullptr)
        {
            DECODE_CHK_STATUS(m_allocator->Destroy(m_kernelBinBuffer));
        }
        DECODE_CHK_STATUS(VvcDecodeS2LPkt::Destroy());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS VvcDecodeS2LPktXe3P_Lpm_Base::AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        uint32_t index = HucKernelSource::vvcS2lKernelId;
        if (m_isPpgttMode)
        {
            DECODE_CHK_STATUS(m_hucKernelSource->GetKernelHashIdx(HucKernelSource::vvcS2lKernelId, index));
        }

        auto &par            = m_hucItf->MHW_GETPAR_F(HUC_IMEM_STATE)();
        par                  = {};
        par.kernelDescriptor = index;
        DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_IMEM_STATE)(&cmdBuffer));

        auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
        mfxWaitParams                     = {};
        mfxWaitParams.iStallVdboxPipeline = true;
        DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

        return MOS_STATUS_SUCCESS;
    }

    uint32_t VvcDecodeS2LPktXe3P_Lpm_Base::GetSliceBatchOffset(uint32_t sliceNum)
    {
        DECODE_FUNC_CALL();
        uint32_t sliceLvlBufSize = 0;
        VvcPipelineXe3P_Lpm_Base *pipeline        = dynamic_cast<VvcPipelineXe3P_Lpm_Base *>(m_vvcPipeline);
        if (pipeline != nullptr && pipeline->m_vvcSlicePkt != nullptr)
        {
            sliceLvlBufSize = MOS_ALIGN_CEIL(pipeline->m_vvcSlicePkt->GetSliceStatesSize(), 64);
        }
        else
        {
            DECODE_ASSERTMESSAGE("invalid pipeline or packet.");
        }
        return sliceLvlBufSize * sliceNum;
    }

    MHW_SETPAR_DECL_SRC(HUC_IMEM_ADDR, VvcDecodeS2LPktXe3P_Lpm_Base)
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_kernelBinBuffer);

        params.kernelbinOffset = 0;
        params.kernelBinBuffer = &(m_kernelBinBuffer->OsResource);
        params.kernelBinSize   = m_kernelBinBuffer->size;

        if (m_osInterface->osCpInterface != nullptr && m_osInterface->osCpInterface->IsHMEnabled())
        {
            params.integrityEnable = true;
        }
        else
        {
            params.integrityEnable = false;
        }

        return MOS_STATUS_SUCCESS;
    }
};
