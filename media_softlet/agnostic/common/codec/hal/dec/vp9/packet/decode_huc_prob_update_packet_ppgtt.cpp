/*
* Copyright (c) 2024-2026, Intel Corporation
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
//! \file     decode_huc_prob_update_packet_ppgtt.cpp
//! \brief    Defines the interface for huc prob update ppgtt packet for VP9 decode
//!
#include "decode_huc_prob_update_packet_ppgtt.h"
#include "mhw_vdbox.h"
#include "decode_resource_auto_lock.h"
#include "mos_os_cp_interface_specific.h"
#include "media_interfaces_huc_kernel_source.h"

namespace decode
{
MOS_STATUS HucVp9ProbUpdatePktPpgtt::AllocateResources()
{
    DECODE_CHK_STATUS(HucVp9ProbUpdatePkt::AllocateResources());

    if (m_isPpgttMode && m_kernelBinBuffer == nullptr)
    {
        DECODE_CHK_NULL(m_hucKernelSource);
        HucKernelSource::HucBinary hucBinary;
        DECODE_CHK_STATUS(m_hucKernelSource->GetKernelBin(HucKernelSource::vp9ProbUpdateKernelId, hucBinary));
        DECODE_CHK_NULL(hucBinary.m_data);

        m_kernelBinBuffer = m_allocator->AllocateBuffer(
            MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE), "Vp9ProbUpdateKernelBinBuffer",
            resourceInternalReadWriteCache, lockableVideoMem);
        DECODE_CHK_NULL(m_kernelBinBuffer);

        ResourceAutoLock kernelBinResLock(m_allocator, &m_kernelBinBuffer->OsResource);
        void *kernelBinBuffer = kernelBinResLock.LockResourceForWrite();
        DECODE_CHK_NULL(kernelBinBuffer);

        MOS_SecureMemcpy(kernelBinBuffer, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
    }

    return MOS_STATUS_SUCCESS;
}

HucVp9ProbUpdatePktPpgtt::~HucVp9ProbUpdatePktPpgtt()
{
    if (m_hucKernelSource != nullptr && m_pipeline != nullptr)
    {
        m_hucKernelSource->ReportMode(m_pipeline->GetSkuTable(), m_pipeline->GetUserSetting());
    }

    if (m_allocator != nullptr && m_kernelBinBuffer != nullptr)
    {
        m_allocator->Destroy(m_kernelBinBuffer);
    }
}

MOS_STATUS HucVp9ProbUpdatePktPpgtt::Init()
{
    DECODE_FUNC_CALL();

    m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
    DECODE_CHK_NULL(m_hucKernelSource);
    DECODE_CHK_NULL(m_pipeline);
    m_isPpgttMode = m_hucKernelSource->IsPpgttMode(m_pipeline->GetSkuTable(), m_pipeline->GetUserSetting());

    DECODE_CHK_STATUS(HucVp9ProbUpdatePkt::Init());

    if (m_isPpgttMode)
    {
        std::shared_ptr<mhw::vdbox::huc::ItfPPGTT> itf =
            std::dynamic_pointer_cast<mhw::vdbox::huc::ItfPPGTT>(m_hucItf);
        DECODE_CHK_COND(!itf, "cannot convert huc itf to Huc ext itf!");
        m_pictureStatesSize += itf->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
        m_picturePatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktPpgtt::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    if (m_isPpgttMode)
    {
        std::shared_ptr<mhw::vdbox::huc::ItfPPGTT> itf =
            std::dynamic_pointer_cast<mhw::vdbox::huc::ItfPPGTT>(m_hucItf);
        DECODE_CHK_COND(!itf, "cannot convert huc itf to Huc ext itf!");

        SETPAR_AND_ADDCMD(HUC_IMEM_ADDR, itf, &cmdBuffer);
    }

    DECODE_CHK_STATUS(AddCmd_HUC_IMEM_STATE(cmdBuffer));
    DECODE_CHK_STATUS(AddCmd_HUC_PIPE_MODE_SELECT(cmdBuffer));

    SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, &cmdBuffer);
    SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, &cmdBuffer);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HucVp9ProbUpdatePktPpgtt::AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    uint32_t index = HucKernelSource::vp9ProbUpdateKernelId;
    if(m_isPpgttMode)
    {
        DECODE_CHK_NULL(m_hucKernelSource);
        DECODE_CHK_STATUS(m_hucKernelSource->GetKernelHashIdx(
            HucKernelSource::vp9ProbUpdateKernelId, index));
    }

    auto &par = m_hucItf->MHW_GETPAR_F(HUC_IMEM_STATE)();
    par                  = {};
    par.kernelDescriptor = index;
    DECODE_CHK_STATUS(m_hucItf->MHW_ADDCMD_F(HUC_IMEM_STATE)(&cmdBuffer));

    auto &mfxWaitParams               = m_miItf->MHW_GETPAR_F(MFX_WAIT)();
    mfxWaitParams                     = {};
    mfxWaitParams.iStallVdboxPipeline = true;
    DECODE_CHK_STATUS((m_miItf->MHW_ADDCMD_F(MFX_WAIT)(&cmdBuffer)));

    return MOS_STATUS_SUCCESS;
}

MHW_SETPAR_DECL_SRC(HUC_IMEM_ADDR, HucVp9ProbUpdatePktPpgtt)
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

}  // namespace decode

