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
//! \file     encode_hevc_huc_brc_update_packet_xe3p_lpm_base.cpp
//! \brief    Defines the implementation of huc update packet 
//!

#include "encode_hevc_huc_brc_update_packet_xe3p_lpm_base.h"
#include "encode_hevc_basic_feature_xe3p_lpm_base.h"
#include "media_interfaces_huc_kernel_source.h"
#if _KERNEL_RESERVED
#include "encode_saliency_feature.h"
#endif

namespace encode
{
    MOS_STATUS HevcHucBrcUpdatePktXe3p_Lpm_Base::Init()
    {
        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_hwInterface);
        m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
        ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
        ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->Init(m_hwInterface->GetSkuTable(), m_userSettingPtr));
        m_isPPGTT = m_hucKernelSource->IsPpgttMode();

        // Save xe3p-specific size constants before parent Init() overwrites them.
        // Parent HucBrcUpdatePkt::Init() recalculates these with a formula that
        // does not include VDENC_HEVC_VP9_TILE_SLICE_STATE, which xe3p requires.
        uint32_t saved_ReadBatchBufferSize     = m_hwInterface->m_vdencReadBatchBufferSize;
        uint32_t saved_2ndLevelBBSize          = m_hwInterface->m_vdenc2ndLevelBatchBufferSize;
        uint32_t saved_PerSliceConstSize       = m_hwInterface->m_vdencBatchBufferPerSliceConstSize;

        ENCODE_CHK_STATUS_RETURN(HucBrcUpdatePkt::Init());

        // Restore xe3p values.  The cacheline-aligned 1stGroupSize / 2ndGroupSize
        // from parent Init are kept (needed for proper SLBB alignment).
        m_hwInterface->m_vdencReadBatchBufferSize          = saved_ReadBatchBufferSize;
        m_hwInterface->m_vdenc2ndLevelBatchBufferSize      = saved_2ndLevelBBSize;
        m_hwInterface->m_vdencBatchBufferPerSliceConstSize = saved_PerSliceConstSize;

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcHucBrcUpdatePktXe3p_Lpm_Base::AllocateResources()
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(EncodeHucPkt::AllocateResources());

        // Get xe3p_lpm_base basic feature for SLBB access
        auto basicFeature = dynamic_cast<HevcBasicFeatureXe3p_Lpm_Base *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(basicFeature);

        // Inline-expand HucBrcUpdatePkt::AllocateResources() logic
        MOS_ALLOC_GFXRES_PARAMS allocParamsForBufferLinear;
        MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
        allocParamsForBufferLinear.Type = MOS_GFXRES_BUFFER;
        allocParamsForBufferLinear.TileType = MOS_TILE_LINEAR;
        allocParamsForBufferLinear.Format = Format_Buffer;

        // HuC FW Region 6: Data Buffer of Current Picture
        // Data (1024 bytes) for current
        // Data (1024 bytes) for ref0
        // Data (1024 bytes) for ref1
        // Data (1024 bytes) for ref2
        allocParamsForBufferLinear.dwBytes = CODECHAL_PAGE_SIZE * 4;
        allocParamsForBufferLinear.pBufName = "Data from Pictures Buffer for Weighted Prediction";
        allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
        MOS_RESOURCE *allocatedbuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
        ENCODE_CHK_NULL_RETURN(allocatedbuffer);
        m_dataFromPicsBuffer = *allocatedbuffer;

        for (auto k = 0; k < CODECHAL_ENCODE_RECYCLED_BUFFER_NUM; k++)
        {
            // Const Data buffer
            allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcConstDataBufferSize, CODECHAL_PAGE_SIZE);
            allocParamsForBufferLinear.pBufName = "VDENC BRC Const Data Buffer";
            allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_WRITE;
            allocatedbuffer       = m_allocator->AllocateResource(allocParamsForBufferLinear, true);
            ENCODE_CHK_NULL_RETURN(allocatedbuffer);
            m_vdencBrcConstDataBuffer[k] = *allocatedbuffer;

            // BRC HuC reads the SLBB Update output (2ndLevelBatchBuffer) as Region 3
            // instead of Origin, because the SLBB Update HuC has already filled in
            // field values into the 2ndLevelBatchBuffer.  Reading Origin
            // would give the BRC HuC a template with zeroed sensitive fields.
            MHW_BATCH_BUFFER *batchBuf2ndLevel = basicFeature->GetVdenc2ndLevelBatchBuffer(k);
            ENCODE_CHK_NULL_RETURN(batchBuf2ndLevel);

            for (auto i = 0; i < VDENC_BRC_NUM_OF_PASSES; i++)
            {
                m_vdencReadBatchBufferOrigin[k][i] = batchBuf2ndLevel->OsResource;

                MHW_BATCH_BUFFER *batchBufferTU7 = basicFeature->GetVdenc2ndLevelBatchBufferTU7(k);
                ENCODE_CHK_NULL_RETURN(batchBufferTU7);
                m_vdencReadBatchBufferTU7[k][i] = batchBufferTU7->OsResource;

                // BRC update DMEM
                allocParamsForBufferLinear.dwBytes = MOS_ALIGN_CEIL(m_vdencBrcUpdateDmemBufferSize, CODECHAL_CACHELINE_SIZE);
                allocParamsForBufferLinear.pBufName = "VDENC BrcUpdate DmemBuffer";
                allocParamsForBufferLinear.ResUsageType = MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ_WRITE_NOCACHE;
                allocatedbuffer                     = m_allocator->AllocateResource(allocParamsForBufferLinear, true, MOS_HW_RESOURCE_USAGE_ENCODE_INTERNAL_READ);
                ENCODE_CHK_NULL_RETURN(allocatedbuffer);
                m_vdencBrcUpdateDmemBuffer[k][i] = *allocatedbuffer;
            }
        }

        // PPGTT kernel binary allocation
        if (m_isPPGTT && m_kernelBinBuffer == nullptr)
        {
            HucKernelSource::HucBinary hucBinary{};
            ENCODE_CHK_NULL_RETURN(m_hucKernelSource);
            ENCODE_CHK_STATUS_RETURN(m_hucKernelSource->GetKernelBin(HucKernelSource::hevcBrcUpdateKernelId, hucBinary));

            ENCODE_CHK_NULL_RETURN(hucBinary.m_data);

            // initiate allocation paramters
            MOS_ZeroMemory(&allocParamsForBufferLinear, sizeof(MOS_ALLOC_GFXRES_PARAMS));
            allocParamsForBufferLinear.Type               = MOS_GFXRES_BUFFER;
            allocParamsForBufferLinear.TileType           = MOS_TILE_LINEAR;
            allocParamsForBufferLinear.Format             = Format_Buffer;
            allocParamsForBufferLinear.dwBytes            = MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE);
            allocParamsForBufferLinear.pBufName           = "HevcUpdateKernelBinBuffer";
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

    MOS_STATUS HevcHucBrcUpdatePktXe3p_Lpm_Base::Execute(PMOS_COMMAND_BUFFER cmdBuffer, bool storeHucStatus2Needed, bool prologNeeded, HuCFunction function)
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
        auto &flushDwParams                         = m_miItf->MHW_GETPAR_F(MI_FLUSH_DW)();
        flushDwParams                               = {};
        flushDwParams.bVideoPipelineCacheInvalidate = true;
        HUC_CHK_STATUS_RETURN(m_miItf->MHW_ADDCMD_F(MI_FLUSH_DW)(cmdBuffer));

        ENCODE_CHK_STATUS_RETURN(EndPerfCollect(*cmdBuffer));
        HUC_CHK_STATUS_RETURN(StoreHuCStatusRegister(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcHucBrcUpdatePktXe3p_Lpm_Base::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        // In the xe3p flow, SLBB Update runs before BRC Update and:
        //   1) Constructs the Origin SLBB template (always includes TILE_SLICE_STATE)
        //   2) Runs the SLBB Update HuC which fills in field values
        //      and writes the result to the 2ndLevelBatchBuffer
        //   3) Saves SLBB layout offsets to HevcBasicFeatureXe3p_Lpm_Base
        //
        // BRC Update reads the 2ndLevelBatchBuffer as its input SLBB (Region 3), 
        // and uses the shared SLBB layout
        // offsets to populate its DMEM.  This avoids calling ConstructBatchBufferHuCBRC
        // which would overwrite Origin with a different SLBB layout (missing
        // TILE_SLICE_STATE when AdaptiveTU is disabled) and use template values
        // instead of the real values.

        ENCODE_FUNC_CALL();

        ENCODE_CHK_NULL_RETURN(m_basicFeature);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_recycleBuf);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcPicParams);
        ENCODE_CHK_NULL_RETURN(m_basicFeature->m_hevcSeqParams);

        ENCODE_CHK_STATUS_RETURN(SetTcbrcMode());

        bool firstTaskInPhase = packetPhase & firstPacket;
        bool requestProlog = false;

        auto brcFeature = dynamic_cast<HEVCEncodeBRC *>(m_featureManager->GetFeature(HevcFeatureIDs::hevcBrcFeature));
        ENCODE_CHK_NULL_RETURN(brcFeature);

        // Read SLBB layout offsets from basic feature (populated by SLBB Update)
        auto basicFeatureXe3p = dynamic_cast<HevcBasicFeatureXe3p_Lpm_Base *>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
        ENCODE_CHK_NULL_RETURN(basicFeatureXe3p);
        m_cmd2StartInBytes              = basicFeatureXe3p->m_slbbCmd2StartInBytes;
        m_slbDataSizeInBytes            = basicFeatureXe3p->m_slbbSlbDataSizeInBytes;
        m_hcpSliceStateCmdSize          = basicFeatureXe3p->m_slbbHcpSliceStateCmdSize;
        m_hcpWeightOffsetStateCmdSize   = basicFeatureXe3p->m_slbbHcpWeightOffsetStateCmdSize;
        m_vdencWeightOffsetStateCmdSize = basicFeatureXe3p->m_slbbVdencWeightOffsetStateCmdSize;
        m_miBatchBufferEndCmdSize       = basicFeatureXe3p->m_slbbMiBatchBufferEndCmdSize;
        MOS_SecureMemcpy(m_alignSize, sizeof(m_alignSize),
                         basicFeatureXe3p->m_slbbAlignSize, sizeof(basicFeatureXe3p->m_slbbAlignSize));

#if _KERNEL_RESERVED
        // Mirror the saliency QP-offset assignment that the base BRC Update path
        // performs in ConstructGroup2Cmds().
        auto saliencyFeature = dynamic_cast<EncodeSaliencyFeature *>(
            m_featureManager->GetFeature(FeatureIDs::saliencyFeature));
        if (saliencyFeature)
        {
            saliencyFeature->m_brcQpOffset = m_cmd2StartInBytes;
        }
#endif

        // TU7 buffer construction is handled by HEVCHucSLBBUpdatePkt::ConstructBatchBuffer
        // (with RDOQ disabled and TU=7).
        // BRC reads the TU7 output via m_vdencReadBatchBufferTU7 (Region 12).

        uint16_t perfTag = m_pipeline->IsFirstPass() ? CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE : CODECHAL_ENCODE_PERFTAG_CALL_BRC_UPDATE_SECOND_PASS;
        uint16_t pictureType = m_basicFeature->m_pictureCodingType;
        if (m_basicFeature->m_pictureCodingType == B_TYPE && m_basicFeature->m_ref.IsLowDelay())
        {
            pictureType = 0;
        }
        SetPerfTag(perfTag, (uint16_t)m_basicFeature->m_mode, pictureType);

        if (!m_pipeline->IsSingleTaskPhaseSupported() || firstTaskInPhase)
        {
            // Send command buffer header at the beginning (OS dependent)
            requestProlog = true;
        }

        // Refresh Region 3 (Origin) and Region 12 (TU7) buffer references from
        // the live basic feature batch buffers right before Execute() builds the
        // HUC_VIRTUAL_ADDR_STATE command.  The value copies made during
        // AllocateResources() may reference stale MOS_RESOURCE metadata.
        {
            int32_t currentPass = m_pipeline->GetCurrentPass();
            const uint32_t bufIdx = m_pipeline->m_currRecycledBufIdx;
            if (currentPass >= 0)
            {
                auto batchBuf = basicFeatureXe3p->GetVdenc2ndLevelBatchBuffer(bufIdx);
                ENCODE_CHK_NULL_RETURN(batchBuf);
                m_vdencReadBatchBufferOrigin[bufIdx][currentPass] = batchBuf->OsResource;

                if (m_basicFeature->m_hevcPicParams->AdaptiveTUEnabled != 0)
                {
                    auto batchBufTU7 = basicFeatureXe3p->GetVdenc2ndLevelBatchBufferTU7(bufIdx);
                    ENCODE_CHK_NULL_RETURN(batchBufTU7);
                    m_vdencReadBatchBufferTU7[bufIdx][currentPass] = batchBufTU7->OsResource;
                }
            }
        }

        ENCODE_CHK_STATUS_RETURN(Execute(commandBuffer, true, requestProlog, BRC_UPDATE));

#if _SW_BRC
        if (!m_swBrc || !m_swBrc->SwBrcEnabled())
        {
#endif
            // Write HUC_STATUS mask: DW1 (mask value)
            auto &storeDataParams            = m_miItf->MHW_GETPAR_F(MI_STORE_DATA_IMM)();
            storeDataParams                  = {};
            storeDataParams.pOsResource      = m_basicFeature->m_recycleBuf->GetBuffer(VdencBrcPakMmioBuffer, 0);
            storeDataParams.dwResourceOffset = sizeof(uint32_t);
            storeDataParams.dwValue          = CODECHAL_VDENC_HEVC_BRC_HUC_STATUS_REENCODE_MASK;
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
#if _SW_BRC
        }
#endif

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HevcHucBrcUpdatePktXe3p_Lpm_Base::CalculateCommandSize(uint32_t &commandBufferSize, uint32_t &requestedPatchListSize)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_STATUS_RETURN(HucBrcUpdatePkt::CalculateCommandSize(commandBufferSize, requestedPatchListSize));

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
