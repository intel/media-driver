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
//! \file     decode_av1_packet_xe2_lpm_base.cpp
//! \brief    Defines the interface for av1 decode packet of Xe2_LPM+
//!
#include "decode_av1_packet_xe2_lpm_base.h"
#include "decode_utils.h"
#include "decode_av1_pipeline.h"
#include "decode_av1_basic_feature.h"
#include "decode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "decode_status_report_defs.h"
#include "decode_resource_auto_lock.h"
#include "hal_oca_interface_next.h"

namespace decode
{
    MOS_STATUS Av1DecodePktXe2_Lpm_Base::Submit(
        MOS_COMMAND_BUFFER* cmdBuffer,
        uint8_t packetPhase)
    {
        DECODE_FUNC_CALL()

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        DECODE_CHK_NULL(cmdBuffer);
        DECODE_CHK_NULL(m_hwInterface);

        int16_t tileIdx = m_av1BasicFeature->m_tileCoding.m_curTile;
        m_isLastTileInPartialFrm = (tileIdx == int16_t(m_av1BasicFeature->m_tileCoding.m_lastTileId)) ? 1 : 0;
        m_isFirstTileInPartialFrm = (tileIdx == int16_t(m_av1BasicFeature->m_tileCoding.m_lastTileId
            - m_av1BasicFeature->m_tileCoding.m_numTiles + 1)) ? 1 : 0;
        
        // For tile missing scenario and duplicate tile scenario
        if (m_av1BasicFeature->m_tileCoding.m_hasTileMissing || m_av1BasicFeature->m_tileCoding.m_hasDuplicateTile)
        {
            m_isFirstTileInPartialFrm = (tileIdx == int16_t(m_av1BasicFeature->m_tileCoding.m_lastTileId - m_av1BasicFeature->m_tileCoding.m_totalTileNum + 1)) ? 1 : 0;
        }

        if (m_isFirstTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse() ||
            (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))
        {
            DECODE_CHK_STATUS(m_miItf->SetWatchdogTimerThreshold(m_av1BasicFeature->m_width, m_av1BasicFeature->m_height, false, CODECHAL_AV1));
            DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_av1BasicFeature->m_destSurface));

            DECODE_CHK_NULL(m_hwInterface->GetVdencInterfaceNext());
            auto mmioRegisters = m_hwInterface->GetVdencInterfaceNext()->GetMmioRegisters(MHW_VDBOX_NODE_1);
            HalOcaInterfaceNext::On1stLevelBBStart(*cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, m_miItf, *mmioRegisters);
            HalOcaInterfaceNext::OnDispatch(*cmdBuffer, *m_osInterface, m_miItf, *m_miItf->GetMmioRegisters());
        }

        DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));
        DECODE_CHK_STATUS(PackTileLevelCmds(*cmdBuffer));

        if (m_isLastTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse() ||
            (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))
        {
            HalOcaInterfaceNext::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);
        }

        if (m_isFirstTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse() ||
            (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))
        {
            DECODE_CHK_STATUS(m_allocator->SyncOnResource(&m_av1BasicFeature->m_resDataBuffer, false));
        }

        m_av1BasicFeature->m_tileCoding.m_curTile++; //Update tile index of current frame

        //Set ReadyToExecute to true for the last tile of the frame
        Mos_Solo_SetReadyToExecute(m_osInterface, m_av1BasicFeature->m_frameCompletedFlag);

        DECODE_CHK_STATUS(Mos_Solo_PostProcessDecode(m_osInterface, &m_av1BasicFeature->m_destSurface));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktXe2_Lpm_Base::VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL()

        auto &par = m_miItf->GETPAR_VD_CONTROL_STATE();
        par       = {};
        par.memoryImplicitFlush = true;
        par.avpEnabled          = true;

        DECODE_CHK_STATUS(m_miItf->ADDCMD_VD_CONTROL_STATE(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktXe2_Lpm_Base::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL()

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        if (m_isFirstTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse() ||
            (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))
        {
            if (IsPrologRequired())
            {
                DECODE_CHK_STATUS(AddForceWakeup(cmdBuffer));
                DECODE_CHK_STATUS(SendPrologWithFrameTracking(cmdBuffer, true));
            }
            DECODE_CHK_STATUS(StartStatusReport(statusReportMfx, &cmdBuffer));
        }

        DECODE_CHK_STATUS(m_picturePkt->InitAv1State(cmdBuffer));

        // For multiple tiles per frame case, picture level command is same between different tiles, so put them into 2nd
        // level BB to exectue picture level cmd only once for 1st tile of the frame and reduce SW latency eventually.
        if (!(m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile) && !m_av1Pipeline->TileBasedDecodingInuse() &&
            !m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            if (m_isFirstTileInPartialFrm)
            {
                m_batchBuf = m_secondLevelBBArray->Fetch();

                if (m_batchBuf != nullptr)
                {
                    ResourceAutoLock resLock(m_allocator, &m_batchBuf->OsResource);
                    uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                    DECODE_CHK_STATUS(InitPicLevelCmdBuffer(*m_batchBuf, batchBufBase));
                    HalOcaInterfaceNext::OnSubLevelBBStart(cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, &m_batchBuf->OsResource, 0, true, 0);
                    m_picCmdBuffer.cmdBuf1stLvl = &cmdBuffer;
                    DECODE_CHK_STATUS(m_picturePkt->Execute(m_picCmdBuffer));
                    DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&m_picCmdBuffer, nullptr));
                }
            }

            DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_BATCH_BUFFER_START(&cmdBuffer, m_batchBuf));
        }
        else
        {
            DECODE_CHK_STATUS(m_picturePkt->Execute(cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktXe2_Lpm_Base::PackTileLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL()

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        int16_t tileIdx = m_av1BasicFeature->m_tileCoding.m_curTile;

        if ( tileIdx < int16_t(m_av1BasicFeature->m_tileCoding.m_numTiles))
        {
            DECODE_CHK_STATUS(m_tilePkt->Execute(cmdBuffer, tileIdx));
        }

        if (m_isLastTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse() ||
            (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))
        {
            DECODE_CHK_STATUS(VdMemoryFlush(cmdBuffer));
            DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
            DECODE_CHK_STATUS(EnsureAllCommandsExecuted(cmdBuffer));
            DECODE_CHK_STATUS(EndStatusReport(statusReportMfx, &cmdBuffer));
        }

        bool isLastTileInFullFrm = (tileIdx == int16_t(m_av1BasicFeature->m_tileCoding.m_numTiles) - 1) ? 1 : 0;

        if (isLastTileInFullFrm)
        {
            DECODE_CHK_STATUS(UpdateStatusReportNext(statusReportGlobalCount, &cmdBuffer));
        }

#ifdef _MMC_SUPPORTED
        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) {
                if (m_av1BasicFeature->m_filmGrainEnabled)
                {
                    m_mmcState->UpdateUserFeatureKey(m_av1BasicFeature->m_filmGrainProcParams->m_outputSurface);
                }
                else
                {
                    m_mmcState->UpdateUserFeatureKey(&(m_av1BasicFeature->m_destSurface));
                }
            })
#endif

        if ((isLastTileInFullFrm || !m_av1Pipeline->FrameBasedDecodingInUse()) &&
             !m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktXe2_Lpm_Base::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL()

        // Send MI_FLUSH command
        auto &par = m_miItf->GETPAR_MI_FLUSH_DW();
        par       = {};
        auto *skuTable = m_av1Pipeline->GetSkuTable();
        if (skuTable && MEDIA_IS_SKU(skuTable, FtrEnablePPCFlush))
        {
            // Add PPC fulsh
            par.bEnablePPCFlush = true;
        }
        DECODE_CHK_STATUS(m_miItf->ADDCMD_MI_FLUSH_DW(&cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

}

