/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     decode_hevc_packet_real_tile_m12.cpp
//! \brief    Defines the interface for hevc real tile decode packet of M12
//!
#include "decode_hevc_packet_real_tile_m12.h"
#include "decode_utils.h"
#include "decode_hevc_pipeline.h"
#include "decode_hevc_basic_feature.h"
#include "decode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "decode_status_report_defs.h"
#include "decode_resource_auto_lock.h"
#include "hal_oca_interface.h"
#include "mhw_vdbox_vdenc_g12_X.h"

namespace decode
{

MOS_STATUS HevcDecodeRealTilePktM12::Init()
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(HevcDecodeRealTilePktXe_M_Base::Init());

    DecodeSubPacket* subPacket = m_hevcPipeline->GetSubPacket(DecodePacketId(m_hevcPipeline, hevcTileSubPacketId));
    m_tilePkt = dynamic_cast<HevcDecodeTilePktM12*>(subPacket);
    DECODE_CHK_NULL(m_tilePkt);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeRealTilePktM12::Submit(
    MOS_COMMAND_BUFFER* cmdBuffer,
    uint8_t packetPhase)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_NULL(m_hwInterface);

    DECODE_CHK_STATUS(m_picturePkt->SetPhase(m_phase));

    DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_hevcBasicFeature->m_destSurface));

    DECODE_CHK_STATUS(m_miInterface->SetWatchdogTimerThreshold(m_hevcBasicFeature->m_width, m_hevcBasicFeature->m_height, false));

    if (IsPrologRequired())
    {
        DECODE_CHK_STATUS(AddForceWakeup(*cmdBuffer));
        DECODE_CHK_STATUS(SendPrologWithFrameTracking(*cmdBuffer, true));
    }

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);
    HalOcaInterface::On1stLevelBBStart(*cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    HalOcaInterface::OnDispatch(*cmdBuffer, *m_osInterface, *m_miInterface, *m_miInterface->GetMmioRegisters());

    auto scalability = m_hevcPipeline->GetMediaScalability();
    DECODE_ASSERT(scalability != nullptr);

    if(m_hevcPipeline->IsFirstPass())
    {
        DECODE_CHK_STATUS(m_miInterface->AddWatchdogTimerStopCmd(cmdBuffer));
        DECODE_CHK_STATUS(scalability->SyncPipe(syncAllPipes, 0, cmdBuffer));

        if (m_osInterface->trinityPath == TRINITY11_ENABLED)
        {
            MHW_MI_FLUSH_DW_PARAMS flushDwParams;
            MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
            DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(cmdBuffer, &flushDwParams));
        }
    }

    if (m_hevcPipeline->IsShortFormat())
    {
        MOS_RESOURCE* osResource = nullptr;
        uint32_t      offset = 0;

        DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatusMask, osResource, offset));

        // Check HuC_STATUS bit15, HW continue if bit15 > 0, otherwise send COND BB END cmd.
        uint32_t compareOperation = mhw_mi_g12_X::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADGREATERTHANIDD;
        DECODE_CHK_STATUS(m_hwInterface->SendCondBbEndCmd(
                osResource, offset, 0, false, false, compareOperation, cmdBuffer));
    }

    if(m_hevcPipeline->IsFirstPass())
    {
        DECODE_CHK_STATUS(m_miInterface->AddWatchdogTimerStartCmd(cmdBuffer));
    }

    DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));

    // In short format, the slice level commands are programed by Huc firmware, so skip slice level commands programing in driver.
    // In long format, first real tile packet programming the slice level commands for whole frame, remaining real tile
    // packets share the same second level batch buffer with different offset accroding to tile index.
    if (!m_hevcPipeline->IsShortFormat() && m_hevcPipeline->IsFirstPass() && m_hevcPipeline->IsFirstPipe())
    {
        DECODE_CHK_STATUS(PackSliceLevelCmds(*cmdBuffer));
    }

    HalOcaInterface::DumpCodechalParam(*cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, m_hevcPipeline->GetCodechalOcaDumper(), CODECHAL_HEVC);
    HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);

    DECODE_CHK_STATUS(m_allocator->SyncOnResource(&m_hevcBasicFeature->m_resDataBuffer, false));

    DECODE_CHK_STATUS(Mos_Solo_PostProcessDecode(m_osInterface, &m_hevcBasicFeature->m_destSurface));

#if USE_CODECHAL_DEBUG_TOOL
    DECODE_CHK_STATUS(DumpSecondaryCommandBuffer(*cmdBuffer));
#endif

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeRealTilePktM12::VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer)
{
    MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.memoryImplicitFlush = true;

    MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
    DECODE_CHK_NULL(miInterfaceG12);
    DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeRealTilePktM12::VdPipelineFlush(MOS_COMMAND_BUFFER & cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_VDBOX_VD_PIPE_FLUSH_PARAMS_G12 vdpipeFlushParams;
    MOS_ZeroMemory(&vdpipeFlushParams, sizeof(vdpipeFlushParams));
    vdpipeFlushParams.Flags.bWaitDoneHEVC = 1;
    vdpipeFlushParams.Flags.bFlushHEVC = 1;
    vdpipeFlushParams.Flags.bWaitDoneVDCmdMsgParser = 1;
    DECODE_CHK_STATUS(m_vdencInterface->AddVdPipelineFlushCmd(
        &cmdBuffer, (MHW_VDBOX_VD_PIPE_FLUSH_PARAMS*)&vdpipeFlushParams));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeRealTilePktM12::VdScalabPipeUnLock(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
    MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
    vdCtrlParam.scalableModePipeUnlock = true;

    MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
    DECODE_CHK_NULL(miInterfaceG12);
    DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeRealTilePktM12::VdWait(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    return m_miInterface->AddMfxWaitCmd(&cmdBuffer, nullptr, true);
}

MOS_STATUS HevcDecodeRealTilePktM12::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(StartStatusReport(statusReportMfx, &cmdBuffer));

    DECODE_CHK_STATUS(m_picturePkt->Execute(cmdBuffer));

    PMHW_BATCH_BUFFER batchBuffer = m_hevcPipeline->GetSliceLvlCmdBuffer();
    DECODE_CHK_NULL(batchBuffer);
    uint8_t curTileCol = m_hevcPipeline->GetCurrentPipe() + m_hevcPipeline->GetCurrentPass() * m_hevcPipeline->GetPipeNum();
    batchBuffer->dwOffset = curTileCol * batchBuffer->iSize;
    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, batchBuffer));

    DECODE_CHK_STATUS(VdMemoryFlush(cmdBuffer));
    DECODE_CHK_STATUS(VdScalabPipeUnLock(cmdBuffer));
    DECODE_CHK_STATUS(VdWait(cmdBuffer));
    DECODE_CHK_STATUS(ReadVdboxId(cmdBuffer));
    DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));

    if (IsLastTileCol())
    {
        DECODE_CHK_STATUS(MiFlush(cmdBuffer));
        DECODE_CHK_STATUS(EndStatusReport(statusReportMfx, &cmdBuffer));
        DECODE_CHK_STATUS(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));
        DECODE_CHK_STATUS(MiFlush(cmdBuffer));
        DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }
    else
    {
        DECODE_CHK_STATUS(NullHW::StopPredicate(m_osInterface, m_miInterface, &cmdBuffer));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HevcDecodeRealTilePktM12::PackSliceLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    PMHW_BATCH_BUFFER batchBuffer = m_hevcPipeline->GetSliceLvlCmdBuffer();
    DECODE_CHK_NULL(batchBuffer);

    uint32_t tileColNum = m_hevcPicParams->num_tile_columns_minus1 + 1;

    ResourceAutoLock resLock(m_allocator, &batchBuffer->OsResource);
    uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
    DECODE_CHK_NULL(batchBufBase);
    DECODE_CHK_STATUS(InitSliceLevelCmdBuffer(*batchBuffer, batchBufBase, tileColNum));

    for (uint32_t i = 0; i < m_hevcBasicFeature->m_numSlices; i++)
    {
        const HevcTileCoding::SliceTileInfo *sliceTileInfo = m_hevcBasicFeature->m_tileCoding.GetSliceTileInfo(i);
        DECODE_CHK_NULL(sliceTileInfo);

        uint16_t subStreamCount = (sliceTileInfo->numTiles > 0) ? sliceTileInfo->numTiles : 1;
        for (uint16_t j = 0; j < subStreamCount; j++)
        {
            uint16_t tileX, tileY;
            if (sliceTileInfo->numTiles > 1)
            {
                DECODE_CHK_NULL(sliceTileInfo->tileArrayBuf);
                const HevcTileCoding::SubTileInfo &subTileInfo = sliceTileInfo->tileArrayBuf[j];
                tileX = subTileInfo.tileX;
                tileY = subTileInfo.tileY;
            }
            else
            {
                tileX = sliceTileInfo->sliceTileX;
                tileY = sliceTileInfo->sliceTileY;
            }

            DECODE_ASSERT(m_sliceLevelCmdBuffer.size() >= tileX);
            MOS_COMMAND_BUFFER &sliceCmdBuffer = m_sliceLevelCmdBuffer[tileX];

            if (sliceTileInfo->firstSliceOfTile)
            {
                DECODE_CHK_STATUS(m_tilePkt->Execute(sliceCmdBuffer, tileX, tileY));
            }
            DECODE_CHK_STATUS(m_slicePkt->Execute(sliceCmdBuffer, i, j));
            if (MEDIA_IS_WA(m_hevcPipeline->GetWaTable(), Wa_2209620131))
            {
                DECODE_CHK_STATUS(VdWait(sliceCmdBuffer));
                DECODE_CHK_STATUS(VdPipelineFlush(sliceCmdBuffer));
            }
        }
    }

    for (uint32_t i = 0; i < tileColNum; i++)
    {
        auto &sliceCmdBuffer = m_sliceLevelCmdBuffer[i];
        DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&sliceCmdBuffer, nullptr));
    }

    return MOS_STATUS_SUCCESS;
}

}

