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
//! \file     encode_vp9_huc_brc_update_packet_xe3p_lpm_base.cpp
//! \brief    Defines the implementation of huc update packet for VP9
//!

#include "encode_vp9_huc_brc_update_packet_xe3p_lpm_base.h"
#include "media_interfaces_huc_kernel_source.h"

namespace encode
{

MOS_STATUS Vp9HucBrcUpdatePktXe3p_Lpm_Base::Init()
{
    ENCODE_FUNC_CALL();

    m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
    ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
    ENCODE_CHK_NULL_RETURN(m_hwInterface);
    m_isPPGTT = m_hucKernelSource->IsPpgttMode(m_hwInterface->GetSkuTable(), m_userSettingPtr);

    ENCODE_CHK_STATUS_RETURN(Vp9HucBrcUpdatePkt::Init());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcUpdatePktXe3p_Lpm_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9HucBrcUpdatePkt::CalculateCommandSize(commandBufferSize, requestedPatchListSize));

    if (m_isPPGTT)
    {
        ENCODE_CHK_NULL_RETURN(m_itfExt);
        commandBufferSize += m_itfExt->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
        requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfExt::HUC_IMEM_ADDR_CMD);
        commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Vp9HucBrcUpdatePktXe3p_Lpm_Base::AllocateResources()
{
    ENCODE_FUNC_CALL();
    ENCODE_CHK_STATUS_RETURN(Vp9HucBrcUpdatePkt::AllocateResources());

    if (m_isPPGTT && m_kernelBinBuffer == nullptr)
    {
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        HucKernelSource::HucBinary hucBinary{};
        ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::vp9VdencBrcUpdateKernelId, hucBinary));

        ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

        // initiate allocation paramters
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format             = Format_Buffer;
        allocParamsForBufferLinear.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
        allocParamsForBufferLinear.pBufName           = "Vp9BrcUpdateKernelBinBuffer";
        allocParamsForBufferLinear.ResUsageType       = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_CACHE;
        allocParamsForBufferLinear.Flags.bNotLockable = false;  // Resource can be CPU accessed

        m_kernelBinBuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, false);
        auto data         = (uint16_t *)m_allocator->LockResourceForWrite(m_kernelBinBuffer);
        ENCODE_CHK_NULL_RETURN(data);
        MOS_SecureMemcpy(data, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
        ENCODE_CHK_STATUS_RETURN(m_allocator->UnLock(m_kernelBinBuffer));
    }

    return MOS_STATUS_SUCCESS;
}
MOS_STATUS Vp9HucBrcUpdatePktXe3p_Lpm_Base::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
{
    ENCODE_FUNC_CALL();
    HUC_CHK_NULL_RETURN(cmdBuffer);
    ENCODE_CHK_NULL_RETURN(m_itfExt);

#if _SW_BRC
    HUC_CHK_STATUS_RETURN(InitSwBrc(function));
    if (function != NONE_BRC && m_swBrc && m_swBrc->SwBrcEnabled())
    {
        SETPAR(HUC_DMEM_STATE, m_hucItf);
        SETPAR(HUC_VIRTUAL_ADDR_STATE, m_hucItf);

        auto &virtualAddrParams = m_hucItf->MHW_GETPAR_F(HUC_VIRTUAL_ADDR_STATE)();
        auto &dmemParams        = m_hucItf->MHW_GETPAR_F(HUC_DMEM_STATE)();

        CODECHAL_DEBUG_TOOL(
            ENCODE_CHK_STATUS_RETURN(DumpInput());)

        EncodeBasicFeature *basicFeature = dynamic_cast<EncodeBasicFeature *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        HUC_CHK_NULL_RETURN(basicFeature);
        return m_swBrc->SwBrcImpl(
            function,
            virtualAddrParams,
            dmemParams,
            basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0));
    }
#endif  // !_SW_BRC

    if (prologNeeded)
    {
        ENCODE_CHK_STATUS_RETURN(AddForceWakeup(*cmdBuffer));
        ENCODE_CHK_STATUS_RETURN(SendPrologCmds(*cmdBuffer));
    }

    ENCODE_CHK_STATUS_RETURN(StartPerfCollect(*cmdBuffer));

    if (m_isPPGTT)
    {
        SETPAR_AND_ADDCMD(HUC_IMEM_ADDR, m_itfExt, cmdBuffer);
    }
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_IMEM_STATE(cmdBuffer));
    ENCODE_CHK_STATUS_RETURN(AddAllCmds_HUC_PIPE_MODE_SELECT(cmdBuffer));

    SETPAR_AND_ADDCMD(HUC_DMEM_STATE, m_hucItf, cmdBuffer);
    SETPAR_AND_ADDCMD(HUC_VIRTUAL_ADDR_STATE, m_hucItf, cmdBuffer);

    m_enableHucStatusReport = true;
    HUC_CHK_STATUS_RETURN(StoreHuCStatus2Register(cmdBuffer, storeHucStatus2Needed));

    SETPAR_AND_ADDCMD(HUC_START, m_hucItf, cmdBuffer);

    CODECHAL_DEBUG_TOOL(
        ENCODE_CHK_STATUS_RETURN(DumpInput());)

    SETPAR_AND_ADDCMD(VD_PIPELINE_FLUSH, m_vdencItf, cmdBuffer);

    // Flush the engine to ensure memory written out
    ENCODE_CHK_NULL_RETURN(m_miItf);
    auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
    flushDwParams                               = {};
    flushDwParams.bVideoPipelineCacheInvalidate = true;
    HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

    ENCODE_CHK_STATUS_RETURN(EndPerfCollect(*cmdBuffer));
    HUC_CHK_STATUS_RETURN(StoreHuCStatusRegister(cmdBuffer));

    return MOS_STATUS_SUCCESS;
}

}  // namespace encode
