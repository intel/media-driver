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
//! \file     encode_av1_brc_update_packet_xe3p_lpm_base.cpp
//! \brief    Defines the implementation of av1 brc update packet
//!

#include "encode_av1_brc_update_packet_xe3p_lpm_base.h"
#include "encode_av1_basic_feature_xe3p_lpm_base.h"
#include "media_interfaces_huc_kernel_source.h"

namespace encode
{
    MOS_STATUS Av1BrcUpdatePktXe3p_Lpm_Base::Init()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->Init(m_hwInterface->GetSkuTable(), m_userSettingPtr));
        m_isPPGTT = m_hucKernelSource->IsPpgttMode();

        ENCODE_CHK_STATUS_RETURN(Av1BrcUpdatePkt::Init());

        return MOS_STATUS_SUCCESS;
    }
    MOS_STATUS Av1BrcUpdatePktXe3p_Lpm_Base::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        // Get xe3p_lpm_base basic feature for SLBB access
        auto basicFeature = dynamic_cast<Av1BasicFeatureXe3P_Lpm_Base *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(basicFeature);

        // Inline-expand Av1BrcUpdatePkt::AllocateResources() logic
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        MOS_RESOURCE *allocatedbuffer = nullptr;

        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            // Const Data buffer
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcConstDataBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC BRC Const Data Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencBrcConstDataBuffer[k] = *allocatedbuffer;

            // Pak insert buffer (input for HuC FW)
            allocParamsForBufferLinear.dwBytes = CODECHAL_PAGE_SIZE;
            allocParamsForBufferLinear.pBufName = "VDENC Read Batch Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencPakInsertBatchBuffer[k] = *allocatedbuffer;

            for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
            {
                // Use SLBB 2nd level batch buffer from xe3p_lpm_base basic feature instead of allocating here
                MHW_BATCH_BUFFER *secondLevelBatchBuffer = basicFeature->GetVdenc2ndLevelBatchBuffer(k);
                ENCODE_CHK_NULL_RETURN(secondLevelBatchBuffer);
                m_vdencReadBatchBufferOrigin[k][i] = secondLevelBatchBuffer->OsResource;

                MHW_BATCH_BUFFER *batchBufferTU7 = basicFeature->GetVdenc2ndLevelBatchBufferTU7(k);
                ENCODE_CHK_NULL_RETURN(batchBufferTU7);
                m_vdencReadBatchBufferTU7[k][i] = batchBufferTU7->OsResource;

                // BRC update DMEM
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
                allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
                allocatedbuffer = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencBrcUpdateDmemBuffer[k][i] = *allocatedbuffer;
            }
        }

        if (m_isPPGTT && m_kernelBinBuffer == nullptr)
        {
            ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
            HucKernelSource::HucBinary hucBinary{};
            ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::av1BrcUpdateKernelId, hucBinary));

            ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

            // initiate allocation paramters
            MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format             = Format_Buffer;
            allocParamsForBufferLinear.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName           = "Av1UpdateKernelBinBuffer";
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

    MOS_STATUS Av1BrcUpdatePktXe3p_Lpm_Base::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
    {
        ENCODE_FUNC_CALL();
        HUC_CHK_NULL_RETURN(cmdBuffer);
        ENCODE_CHK_NULL_RETURN(m_itfPPGTT);

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
            SETPAR_AND_ADDCMD(HUC_IMEM_ADDR, m_itfPPGTT, cmdBuffer);
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

    MOS_STATUS Av1BrcUpdatePktXe3p_Lpm_Base::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1PicParams);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_av1SeqParams);

        // SLBB construction for main batch buffers (origin and TU7) is now handled by AV1SLBBUpdatePkt
        // Only construct PAK insert batch buffer here
        ENCODE_CHK_STATUS_RETURN(ConstructPakInsertHucBRC(&m_vdencPakInsertBatchBuffer[m_pipeline->m_currRecycledBufIdx]));

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;

        auto brcFeature = dynamic_cast<Av1Brc *>(m_featureManager->GetFeature(Av1FeatureIDs::av1BrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        uint16_t perfTag = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE : CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_SECOND_PASS;
        uint16_t pictureType = (m_basicFeature->m_pictureCodingType == I_TYPE) ? 0 : (m_basicFeature->m_ref.IsLowDelay() ? (m_basicFeature->m_ref.IsPFrame() ? 1 : 3) : 2);
        SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, pictureType);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }

        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, BRC_UPDATE));

        if (!IsHuCStsUpdNeeded())
        {
            // Write HUC_STATUS mask: DW1 (mask value)
            auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeDataParams                  = {};
            storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
            storeDataParams.dwResourceOffset = sizeof(uint32_t);
            storeDataParams.dwValue          = AV1_BRC_HUC_STATUS_REENCODE_MASK;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_DATA_IMM)(commandBuffer));

            // store HUC_STATUS register: DW0 (actual value)
            ENCODE_CHK_COND_RETURN((m_vdboxIndex > MHW_VDBOX_NODE_1), "ERROR - vdbox index exceed the maximum");
            auto mmioRegisters              = m_hucItf->GetMmioRegisters(m_vdboxIndex);
            auto &storeRegParams            = m_miItf->MHW_GETPAR_F(MI_STORE_REGISTER_MEM)();
            storeDataParams                 = {};
            storeRegParams.presStoreBuffer  = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
            storeRegParams.dwOffset         = 0;
            storeRegParams.dwRegister       = mmioRegisters->hucStatusRegOffset;
            ENCODE_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_STORE_REGISTER_MEM)(commandBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1BrcUpdatePktXe3p_Lpm_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(Av1BrcUpdatePkt::CalculateCommandSize(commandBufferSize, requestedPatchListSize));

        if (m_isPPGTT)
        {
            ENCODE_CHK_NULL_RETURN(m_itfPPGTT);
            commandBufferSize += m_itfPPGTT->MHW_GETSIZE_F(HUC_IMEM_ADDR)();
            requestedPatchListSize += PATCH_LIST_COMMAND(mhw::vdbox::huc::ItfPPGTT::HUC_IMEM_ADDR_CMD);
            commandBufferSize = MOS_ALIGN_CEIL(commandBufferSize, CODECHAL_PAGE_SIZE);

        }
        return MOS_STATUS_SUCCESS;
    }
}
