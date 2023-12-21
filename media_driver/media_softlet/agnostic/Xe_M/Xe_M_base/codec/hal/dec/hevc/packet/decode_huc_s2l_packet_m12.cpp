/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     decode_huc_s2l_packet_m12.cpp
//! \brief    Defines the interface for huc S2L packet for M12
//!
#include "decode_huc_s2l_packet_m12.h"
#include "mhw_vdbox.h"
#include "mhw_vdbox_g12_X.h"
#include "mhw_vdbox_vdenc_g12_X.h"
#include "decode_resource_auto_lock.h"

namespace decode {

    MOS_STATUS HucS2lPktM12::AllocateResources()
    {
        DECODE_CHK_STATUS(HucS2lPktXe_M_Base::AllocateResources());

        m_dmemBufferSize = MOS_ALIGN_CEIL(sizeof(HucHevcS2lBssM12), CODECHAL_CACHELINE_SIZE);
        if (m_s2lDmemBufferArray == nullptr)
        {
            m_s2lDmemBufferArray = m_allocator->AllocateBufferArray(
                m_dmemBufferSize, "DmemBuffer", CODECHAL_HEVC_NUM_DMEM_BUFFERS,
                resourceInternalReadWriteCache, lockableVideoMem);
            DECODE_CHK_NULL(m_s2lDmemBufferArray);
        }

        DECODE_CHK_NULL(m_hwInterface);
        MEDIA_WA_TABLE *waTable = m_hwInterface->GetWaTable();
        if (waTable && MEDIA_IS_WA(waTable, WaCheckHucAuthenticationStatus))
        {
            m_hucAuthPkt = MOS_New(DecodeHucAuthCheckPktM12, m_hevcPipeline, m_hwInterface);
        }

        if (m_hucAuthPkt != nullptr)
        {
            // allocate resources for huc auth packet
            DECODE_CHK_STATUS(m_hucAuthPkt->Init());
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::Destroy()
    {
        if (m_hucAuthPkt != nullptr)
        {
            // destroy resources allocated in huc auth packet
            DECODE_CHK_STATUS(m_hucAuthPkt->Destroy());
            MOS_Delete(m_hucAuthPkt);
        }

        DECODE_CHK_STATUS(m_allocator->Destroy(m_s2lDmemBufferArray));
        DECODE_CHK_STATUS(HucS2lPktXe_M_Base::Destroy());
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::Prepare()
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(m_hwInterface);

        DECODE_CHK_STATUS(HucS2lPktXe_M_Base::Prepare());
        DECODE_CHK_STATUS(SetDmemBuffer());

        SetHucStatusMask(m_hucInterface->GetHucStatusHevcS2lFailureMask(),
                         m_hucInterface->GetHucStatus2ImemLoadedMask());

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::Submit(MOS_COMMAND_BUFFER *commandBuffer, uint8_t packetPhase)
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

    MOS_STATUS HucS2lPktM12::Execute(MOS_COMMAND_BUFFER& cmdBuffer, bool prologNeeded)
    {
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(m_hucInterface);

        if (m_hucAuthPkt != nullptr)
        {
            // add huc auth check cmds
            DECODE_CHK_STATUS(m_hucAuthPkt->Execute(cmdBuffer));
        }

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

        MOS_RESOURCE* osResource = nullptr;
        uint32_t     offset = 0;

        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Mask, osResource, offset));

         // Check HuC_STATUS2 bit6, if bit6 > 0 HW continue execution following cmd, otherwise it send a COND BB END cmd.
        uint32_t compareOperation = mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADGREATERTHANIDD;
        DECODE_CHK_STATUS(m_hwInterface->SendCondBbEndCmd(
            osResource, offset, 0, false, false, compareOperation, &cmdBuffer));

        DECODE_CHK_STATUS(StoreHucStatusRegister(cmdBuffer));

        if(!m_pipeline->IsSingleTaskPhaseSupported())
        {
            DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_STATUS(AddHucImem(cmdBuffer));
        DECODE_CHK_STATUS(AddHucPipeModeSelect(cmdBuffer));
        DECODE_CHK_STATUS(AddHucIndObj(cmdBuffer));
        DECODE_CHK_STATUS(AddHucRegion(cmdBuffer));
        DECODE_CHK_STATUS(AddHucDmem(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        for (uint32_t i = 0; i < m_hevcBasicFeature->m_numSlices; i++)
        {
            DECODE_CHK_STATUS(AddHucCpState(cmdBuffer, i, m_hevcSliceParams[i]));
            DECODE_CHK_STATUS(AddHucStreamObject(cmdBuffer, m_hevcSliceParams[i]));
            if (i == (m_hevcBasicFeature->m_numSlices-1))
            {
                DECODE_CHK_STATUS(StoreHucStatus2Register(cmdBuffer));
                DECODE_CHK_STATUS(m_hucInterface->AddHucStartCmd(&cmdBuffer, true));
            }
            else
            {
                DECODE_CHK_STATUS(m_hucInterface->AddHucStartCmd(&cmdBuffer, false));
            }
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::AddHucPipeModeSelect(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_PIPE_MODE_SELECT_PARAMS_G12 pipeModeSelectParams;
        SetHucPipeModeSelectParameters(pipeModeSelectParams);

        DECODE_CHK_STATUS(m_hucInterface->AddHucPipeModeSelectCmd(&cmdBuffer, &pipeModeSelectParams));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::SetHucDmemPictureBss(HucHevcS2lPicBssXe_M_Base &hucHevcS2LPicBss)
    {
        DECODE_CHK_STATUS(HucS2lPktXe_M_Base::SetHucDmemPictureBss(hucHevcS2LPicBss));

        HucHevcS2lPicBssM12 &hucHevcS2LPicBssM12 = static_cast<HucHevcS2lPicBssM12 &>(hucHevcS2LPicBss);

        if (m_hevcRextPicParams)
        {
            hucHevcS2LPicBssM12.high_precision_offsets_enabled_flag =
                m_hevcRextPicParams->PicRangeExtensionFlags.fields.high_precision_offsets_enabled_flag;
            hucHevcS2LPicBssM12.chroma_qp_offset_list_enabled_flag  =
                m_hevcRextPicParams->PicRangeExtensionFlags.fields.chroma_qp_offset_list_enabled_flag;
        }
        else
        {
            hucHevcS2LPicBssM12.high_precision_offsets_enabled_flag = 0;
            hucHevcS2LPicBssM12.chroma_qp_offset_list_enabled_flag  = 0;
        }

        hucHevcS2LPicBssM12.IsRealTileEnable = 0;
        if (m_hevcPipeline->GetDecodeMode() == HevcPipeline::realTileDecodeMode)
        {
            PMHW_BATCH_BUFFER batchBuffer = m_hevcPipeline->GetSliceLvlCmdBuffer();
            DECODE_CHK_NULL(batchBuffer);
            hucHevcS2LPicBssM12.BatchBufferSize   = batchBuffer->iSize;
            hucHevcS2LPicBssM12.NumScalablePipes  = m_pipeline->GetPipeNum();
            hucHevcS2LPicBssM12.IsRealTileEnable  = 1;
        }
        else if (m_hevcPipeline->GetDecodeMode() == HevcPipeline::separateTileDecodeMode)
        {
            hucHevcS2LPicBssM12.NumScalablePipes  = 1;
        }

        hucHevcS2LPicBssM12.IsSCCIBCMode = m_hevcBasicFeature->m_isSCCIBCMode;
        hucHevcS2LPicBssM12.IsSCCPLTMode = m_hevcBasicFeature->m_isSCCPLTMode;
        if (hucHevcS2LPicBssM12.IsSCCIBCMode)
        {
            uint8_t i = 0, k = 0;
            for (i = 0; i < CODEC_MAX_NUM_REF_FRAME_HEVC; i++)
            {
                if (hucHevcS2LPicBssM12.PicOrderCntValList[i] == hucHevcS2LPicBssM12.CurrPicOrderCntVal)
                {
                    break;
                }
            }
            for (k = 0; k < CODECHAL_MAX_CUR_NUM_REF_FRAME_HEVC; k++)
            {
                if (hucHevcS2LPicBssM12.RefPicSetLtCurr[k] == 0xFF)
                {
                    hucHevcS2LPicBssM12.RefPicSetLtCurr[k] = i;
                    break;
                }
            }
        }

        if (hucHevcS2LPicBssM12.IsSCCPLTMode)
        {
            hucHevcS2LPicBssM12.PredictorPaletteSize = m_hevcSccPicParams->PredictorPaletteSize;
            MOS_SecureMemcpy(hucHevcS2LPicBssM12.PredictorPaletteEntries,
                sizeof(hucHevcS2LPicBssM12.PredictorPaletteEntries),
                m_hevcSccPicParams->PredictorPaletteEntries,
                sizeof(m_hevcSccPicParams->PredictorPaletteEntries));
        }
        else
        {
            hucHevcS2LPicBssM12.PredictorPaletteSize = 0;
            MOS_ZeroMemory(hucHevcS2LPicBssM12.PredictorPaletteEntries, sizeof(hucHevcS2LPicBssM12.PredictorPaletteEntries));
        }

        if (m_hevcSccPicParams)
        {
            hucHevcS2LPicBssM12.UseSliceACTOffset = m_hevcSccPicParams->PicSCCExtensionFlags.fields.pps_slice_act_qp_offsets_present_flag;
            hucHevcS2LPicBssM12.pps_act_y_qp_offset = m_hevcSccPicParams->pps_act_y_qp_offset_plus5 - 5;
            hucHevcS2LPicBssM12.pps_act_cb_qp_offset = m_hevcSccPicParams->pps_act_cb_qp_offset_plus5 - 5;
            hucHevcS2LPicBssM12.pps_act_cr_qp_offset = m_hevcSccPicParams->pps_act_cr_qp_offset_plus3 - 3;
            hucHevcS2LPicBssM12.MVRControlIdc = m_hevcSccPicParams->PicSCCExtensionFlags.fields.motion_vector_resolution_control_idc;
        }
        else
        {
            hucHevcS2LPicBssM12.UseSliceACTOffset = 0;
            hucHevcS2LPicBssM12.pps_act_y_qp_offset = 0;
            hucHevcS2LPicBssM12.pps_act_cb_qp_offset = 0;
            hucHevcS2LPicBssM12.pps_act_cr_qp_offset = 0;
            hucHevcS2LPicBssM12.MVRControlIdc = 0;
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HucS2lPktM12::SetDmemBuffer()
    {
        DECODE_FUNC_CALL();

        m_s2lDmemBuffer = m_s2lDmemBufferArray->Fetch();
        DECODE_CHK_NULL(m_s2lDmemBuffer);

        ResourceAutoLock resLock(m_allocator, &m_s2lDmemBuffer->OsResource);
        HucHevcS2lBssM12 *dmemBase = (HucHevcS2lBssM12 *)resLock.LockResourceForWrite();
        DECODE_CHK_NULL(dmemBase);

        dmemBase->ProductFamily       = m_hucInterface->GetHucProductFamily();
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

    MOS_STATUS HucS2lPktM12::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
    {
        DECODE_FUNC_CALL();

        MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12 vdpipeFlushParams;
        MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
        vdpipeFlushParams.Flags.bWaitDoneHEVC           = 1;
        vdpipeFlushParams.Flags.bFlushHEVC              = 1;
        vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
        DECODE_CHK_STATUS(m_vdencInterface->AddVdPipelineFlushCmd(&cmdBuffer, (MHW_VDBOX_VD_PIPE_FLUSH_PARAMS*)&vdpipeFlushParams));

        return MOS_STATUS_SUCCESS;
    }
}
