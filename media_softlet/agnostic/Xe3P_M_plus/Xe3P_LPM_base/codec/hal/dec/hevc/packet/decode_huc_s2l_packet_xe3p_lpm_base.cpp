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
//! \file     decode_huc_s2l_packet_xe3p_lpm_base.cpp
//! \brief    Defines the interface for huc S2L packet for Xe3P_LPM_Base
//!
#include "decode_huc_s2l_packet_xe3p_lpm_base.h"
#include "mhw_vdbox.h"
#include "decode_resource_auto_lock.h"
#include "mhw_vdbox_xe3p_lpm_base.h"
#include "media_interfaces_huc_kernel_source.h"
#include "mos_os_cp_interface_specific.h"

using namespace mhw::vdbox::xe3p_lpm_base;

namespace decode {

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::Init()
    {
        DECODE_FUNC_CALL();
        DECODE_CHK_NULL(m_hevcPipeline);

        m_hucKernelSource = HucKernelSourceDevice::CreateFactory(m_osInterface);
        DECODE_CHK_NULL(m_hucKernelSource);
        m_isPpgttMode = m_hucKernelSource->IsPpgttMode(m_hevcPipeline->GetSkuTable(), m_hevcPipeline->GetUserSetting());

        DECODE_CHK_STATUS(HucS2lPkt::Init());

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

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::AllocateResources()
    {
        DECODE_CHK_STATUS(HucS2lPkt::AllocateResources());

        m_dmemBufferSize = MOS_ALIGN_CEIL(sizeof(HucHevcS2lBssXe3P_Lpm_Base), CODECHAL_CACHELINE_SIZE);
        if (m_s2lDmemBufferArray == nullptr)
        {
            m_s2lDmemBufferArray = m_allocator->AllocateBufferArray(
                m_dmemBufferSize, "DmemBuffer", CODECHAL_HEVC_NUM_DMEM_BUFFERS,
                resourceInternalReadWriteCache, lockableVideoMem);
            DECODE_CHK_NULL(m_s2lDmemBufferArray);
        }

        if (m_isPpgttMode && m_kernelBinBuffer == nullptr)
        {
            HucKernelSource::HucBinary hucBinary;
            DECODE_CHK_STATUS(m_hucKernelSource->GetKernelBin(HucKernelSource::hevcS2lKernelId, hucBinary));
            DECODE_CHK_NULL(hucBinary.m_data);

            m_kernelBinBuffer = m_allocator->AllocateBuffer(
                MOS_ALIGN_CEIL(hucBinary.m_size, CODECHAL_CACHELINE_SIZE), "HevcS2lKernelBinBuffer",
                resourceInternalReadWriteCache, lockableVideoMem);
            DECODE_CHK_NULL(m_kernelBinBuffer);

            ResourceAutoLock kernelBinResLock(m_allocator, &m_kernelBinBuffer->OsResource);
            void *kernelBinBuffer = kernelBinResLock.LockResourceForWrite();
            DECODE_CHK_NULL(kernelBinBuffer);

            MOS_SecureMemcpy(kernelBinBuffer, hucBinary.m_size, hucBinary.m_data, hucBinary.m_size);
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::Destroy()
    {
        if (m_hucKernelSource != nullptr)
        {
            DECODE_CHK_STATUS(m_hucKernelSource->ReportMode(m_hevcPipeline->GetSkuTable(), m_hevcPipeline->GetUserSetting()));
        }

        if (m_allocator != nullptr)
        {
            DECODE_CHK_STATUS(m_allocator->Destroy(m_s2lDmemBufferArray));
            DECODE_CHK_STATUS(m_allocator->Destroy(m_kernelBinBuffer));
        }
        DECODE_CHK_STATUS(HucS2lPkt::Destroy());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::Prepare()
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    
        DECODE_CHK_NULL(m_hwInterface);

        DECODE_CHK_STATUS(HucS2lPkt::Prepare());
        DECODE_CHK_STATUS(SetDmemBuffer());

        SetHucStatusMask(m_hucItf->GetHucStatusHevcS2lFailureMask(),
                         m_hucItf->GetHucStatus2ImemLoadedMask());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    
        DECODE_CHK_NULL(commandBuffer);

        // Send prolog since S2L packet always be first packet 
        bool requestProlog = true;
        DECODE_CHK_STATUS(Execute(*commandBuffer, requestProlog));

        CODECHAL_DEBUG_TOOL
        (
            DECODE_CHK_STATUS(DumpHucS2l());
        )

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::Execute(MOS_COMMAND_BUFFER& cmdBuffer, bool prologNeeded)
    {
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);
    
        DECODE_CHK_NULL(m_hucItf);

        MEDIA_CHK_STATUS_RETURN(NullHW::StartPredicateNext(m_osInterface, m_miItf, &cmdBuffer));
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
        MEDIA_CHK_STATUS_RETURN(NullHW::StopPredicateNext(m_osInterface, m_miItf, &cmdBuffer));
        MOS_RESOURCE* osResource = nullptr;
        uint32_t     offset = 0;

        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Mask, osResource, offset));

         // Check HuC_STATUS2 bit6, if bit6 > 0 HW continue execution following cmd, otherwise it send a COND BB END cmd.
        uint32_t compareOperation = mhw::mi::COMPARE_OPERATION_MADGREATERTHANIDD;
        DECODE_CHK_STATUS(m_hwInterface->SendCondBbEndCmd(
            osResource, offset, 0, false, false, compareOperation, &cmdBuffer));

        DECODE_CHK_STATUS(StoreHucStatusRegister(cmdBuffer));

        if(!m_pipeline->IsSingleTaskPhaseSupported())
        {
            DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
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

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        for (uint32_t i = 0; i < m_hevcBasicFeature->m_numSlices; i++)
        {
            DECODE_CHK_STATUS(AddHucCpState(cmdBuffer, i, m_hevcSliceParams[i]));
            DECODE_CHK_STATUS(HucS2lPkt::AddCmd_HUC_STREAM_OBJECT(cmdBuffer, m_hevcSliceParams[i]));
            if (i == (m_hevcBasicFeature->m_numSlices-1))
            {
                DECODE_CHK_STATUS(StoreHucStatus2Register(cmdBuffer));
                DECODE_CHK_STATUS(HucS2lPkt::AddCmd_HUC_START(cmdBuffer, true));
            }
            else
            {
                DECODE_CHK_STATUS(HucS2lPkt::AddCmd_HUC_START(cmdBuffer, false));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::SetHucDmemPictureBss(HucHevcS2lPicBss &hucHevcS2LPicBss)
    {
        DECODE_CHK_STATUS(HucS2lPkt::SetHucDmemPictureBss(hucHevcS2LPicBss));

        HucHevcS2lPicBssXe3P_Lpm_Base &hucHevcS2lPicBssXe3P_Lpm_Base = static_cast<HucHevcS2lPicBssXe3P_Lpm_Base &>(hucHevcS2LPicBss);

        if (m_hevcRextPicParams)
        {
            hucHevcS2lPicBssXe3P_Lpm_Base.high_precision_offsets_enabled_flag =
                m_hevcRextPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag;
            hucHevcS2lPicBssXe3P_Lpm_Base.chroma_qp_offset_list_enabled_flag  =
                m_hevcRextPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag;
        }
        else
        {
            hucHevcS2lPicBssXe3P_Lpm_Base.high_precision_offsets_enabled_flag = 0;
            hucHevcS2lPicBssXe3P_Lpm_Base.chroma_qp_offset_list_enabled_flag  = 0;
        }

        hucHevcS2lPicBssXe3P_Lpm_Base.IsRealTileEnable = 0;
        if (m_hevcPipeline->GetDecodeMode() == HevcPipeline::realTileDecodeMode)
        {
            PMHW_BATCH_BUFFER batchBuffer = m_hevcPipeline->GetSliceLvlCmdBuffer();
            DECODE_CHK_NULL(batchBuffer);
            hucHevcS2lPicBssXe3P_Lpm_Base.BatchBufferSize   = batchBuffer->iSize;
            hucHevcS2lPicBssXe3P_Lpm_Base.NumScalablePipes  = m_pipeline->GetPipeNum();
            hucHevcS2lPicBssXe3P_Lpm_Base.IsRealTileEnable  = 1;
        }
        else if (m_hevcPipeline->GetDecodeMode() == HevcPipeline::separateTileDecodeMode)
        {
            hucHevcS2lPicBssXe3P_Lpm_Base.NumScalablePipes  = 1;
        }

        hucHevcS2lPicBssXe3P_Lpm_Base.IsSCCIBCMode = m_hevcBasicFeature->m_isSCCIBCMode;
        hucHevcS2lPicBssXe3P_Lpm_Base.IsSCCPLTMode = m_hevcBasicFeature->m_isSCCPLTMode;
        if (hucHevcS2lPicBssXe3P_Lpm_Base.IsSCCIBCMode)
        {
            uint8_t i = 0, k = 0;
            for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                if (hucHevcS2lPicBssXe3P_Lpm_Base.PicOrderCntValList[i] == hucHevcS2lPicBssXe3P_Lpm_Base.CurrPicOrderCntVal)
                {
                    break;
                }
            }
            for (k = 0; k < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; k++)
            {
                if (hucHevcS2lPicBssXe3P_Lpm_Base.RefPicSetLtCurr[k] == 0xFF)
                {
                    hucHevcS2lPicBssXe3P_Lpm_Base.RefPicSetLtCurr[k] = i;
                    break;
                }
            }
        }

        if (hucHevcS2lPicBssXe3P_Lpm_Base.IsSCCPLTMode)
        {
            hucHevcS2lPicBssXe3P_Lpm_Base.PredictorPaletteSize = m_hevcSccPicParams->PredictorPaletteSize;
            MOS_SecureMemcpy(hucHevcS2lPicBssXe3P_Lpm_Base.PredictorPaletteEntries,
                sizeof(hucHevcS2lPicBssXe3P_Lpm_Base.PredictorPaletteEntries),
                m_hevcSccPicParams->PredictorPaletteEntries,
                sizeof(m_hevcSccPicParams->PredictorPaletteEntries));
        }
        else
        {
            hucHevcS2lPicBssXe3P_Lpm_Base.PredictorPaletteSize = 0;
            MOS_ZeroMemory(hucHevcS2lPicBssXe3P_Lpm_Base.PredictorPaletteEntries, sizeof(hucHevcS2lPicBssXe3P_Lpm_Base.PredictorPaletteEntries));
        }

        if (m_hevcSccPicParams)
        {
            hucHevcS2lPicBssXe3P_Lpm_Base.UseSliceACTOffset = m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag;
            hucHevcS2lPicBssXe3P_Lpm_Base.pps_act_y_qp_offset = m_hevcSccPicParams->pps_act_y_qp_offset_plus5 - 5;
            hucHevcS2lPicBssXe3P_Lpm_Base.pps_act_cb_qp_offset = m_hevcSccPicParams->pps_act_cb_qp_offset_plus5 - 5;
            hucHevcS2lPicBssXe3P_Lpm_Base.pps_act_cr_qp_offset = m_hevcSccPicParams->pps_act_cr_qp_offset_plus3 - 3;
            hucHevcS2lPicBssXe3P_Lpm_Base.MVRControlIdc = m_hevcSccPicParams->PicSCCExtensionFlags.fields.motion_vector_resolution_control_idc;
        }
        else
        {
            hucHevcS2lPicBssXe3P_Lpm_Base.UseSliceACTOffset = 0;
            hucHevcS2lPicBssXe3P_Lpm_Base.pps_act_y_qp_offset = 0;
            hucHevcS2lPicBssXe3P_Lpm_Base.pps_act_cb_qp_offset = 0;
            hucHevcS2lPicBssXe3P_Lpm_Base.pps_act_cr_qp_offset = 0;
            hucHevcS2lPicBssXe3P_Lpm_Base.MVRControlIdc = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::SetDmemBuffer()
    {
        DECODE_FUNC_CALL();

        m_s2lDmemBuffer = m_s2lDmemBufferArray->Fetch();
        DECODE_CHK_NULL(m_s2lDmemBuffer);

        ResourceAutoLock resLock(m_allocator, &m_s2lDmemBuffer->OsResource);
        HucHevcS2lBssXe3P_Lpm_Base *dmemBase = (HucHevcS2lBssXe3P_Lpm_Base *)resLock.LockResourceForWrite();
        DECODE_CHK_NULL(dmemBase);

        dmemBase->ProductFamily       = m_hucItf->GetHucProductFamily();
        dmemBase->RevId               = m_hwInterface->GetPlatform().usRevId;
        dmemBase->DummyRefIdxState    = m_hevcBasicFeature->m_useDummyReference && !m_osInterface->bSimIsActive;
        dmemBase->DummyVDControlState = MEDIA_IS_WA(m_hevcPipeline->GetWaTable(), Wa_14010222001);
        dmemBase->WaTileFlushScalability = MEDIA_IS_WA(m_hevcPipeline->GetWaTable(), Wa_2209620131);

        DECODE_CHK_STATUS(SetHucDmemPictureBss(dmemBase->PictureBss));
        DECODE_CHK_STATUS(SetHucDmemSliceBss(dmemBase->SliceBss));

        if (m_hevcBasicFeature->m_numSlices < CODECHAL_HEVC_MAX_NUM_SLICES_LVL_6)
        {
            m_dmemTransferSize = (uint32_t)((uint8_t *)&(dmemBase->SliceBss[m_hevcBasicFeature->m_numSlices]) - 
                                            (uint8_t *)dmemBase);
            m_dmemTransferSize = MOS_ALIGN_CEIL(m_dmemTransferSize, CODECHAL_CACHELINE_SIZE);
        }
        else
        {
            m_dmemTransferSize = m_dmemBufferSize;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
    {
        DECODE_FUNC_CALL();

        auto &par                  = m_vdencItf->GETPAR_VD_PIPELINE_FLUSH();
        par                        = {};
        par.waitDoneHEVC           = 1;
        par.flushHEVC              = 1;
        par.waitDoneVDCmdMsgParser = 1;
        m_vdencItf->ADDCMD_VD_PIPELINE_FLUSH(&cmdBuffer);

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktXe3P_Lpm_Base::AddCmd_HUC_IMEM_STATE(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        uint32_t index = HucKernelSource::hevcS2lKernelId;
        if(m_isPpgttMode)
        {
            DECODE_CHK_STATUS(m_hucKernelSource->GetKernelHashIdx(HucKernelSource::hevcS2lKernelId, index));
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

    MHW_SETPAR_DECL_SRC(HUC_IMEM_ADDR, HucS2lPktXe3P_Lpm_Base)
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
}
