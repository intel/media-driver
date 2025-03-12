/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     decode_av1_packet_g12.cpp
//! \brief    Defines the interface for av1 decode packet of Gen12
//!
#include "decode_av1_packet_g12.h"
#include "decode_utils.h"
#include "decode_av1_pipeline_g12_base.h"
#include "decode_av1_basic_feature_g12.h"
#include "decode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "decode_status_report_defs.h"
#include "decode_resource_auto_lock.h"
#include "hal_oca_interface.h"

namespace decode
{
    MOS_STATUS Av1DecodePktG12::Submit(
        MOS_COMMAND_BUFFER* cmdBuffer,
        uint8_t packetPhase)
    {
        DECODE_FUNC_CALL();

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
            m_isFirstTileInPartialFrm = (tileIdx == int16_t(m_av1BasicFeature->m_tileCoding.m_lastTileId
                - m_av1BasicFeature->m_tileCoding.m_totalTileNum + 1)) ? 1 : 0;
        }
        // For frame based submission mode, these cmds only need to be programed once per frame, otherwise need program per tile.
        if (m_isFirstTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse() ||
            (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))
        {
            DECODE_CHK_STATUS(m_miInterface->SetWatchdogTimerThreshold(
            m_av1BasicFeature->m_width, m_av1BasicFeature->m_height, false));

            DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_av1BasicFeature->m_destSurface));

            auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);
            HalOcaInterface::On1stLevelBBStart(*cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
            HalOcaInterface::OnDispatch(*cmdBuffer, *m_osInterface, *m_miInterface, *m_miInterface->GetMmioRegisters());
        }

        DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));
        DECODE_CHK_STATUS(PackTileLevelCmds(*cmdBuffer));

        if (m_isLastTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse() ||
            (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile))
        {
            HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);
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

    MOS_STATUS Av1DecodePktG12::VdMemoryFlush(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        MHW_MI_VD_CONTROL_STATE_PARAMS vdCtrlParam;
        MOS_ZeroMemory(&vdCtrlParam, sizeof(MHW_MI_VD_CONTROL_STATE_PARAMS));
        vdCtrlParam.memoryImplicitFlush = true;
        vdCtrlParam.avpEnabled          = true;

        MhwMiInterfaceG12* miInterfaceG12 = dynamic_cast<MhwMiInterfaceG12*>(m_miInterface);
        DECODE_CHK_NULL(miInterfaceG12);
        DECODE_CHK_STATUS(miInterfaceG12->AddMiVdControlStateCmd(&cmdBuffer, &vdCtrlParam));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktG12::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

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

        if (m_av1BasicFeature->m_usingDummyWl && ((m_av1Pipeline->TileBasedDecodingInuse() ||
            m_isFirstTileInPartialFrm) || (m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile)))
        {
            DECODE_CHK_STATUS(InitDummyWL(cmdBuffer));
        }

        // For multiple tiles per frame case, picture level command is same between different tiles, so put them into 2nd
        // level BB to exectue picture level cmd only once for 1st tile of the frame and reduce SW latency eventually.
        if (!(m_av1PicParams->m_picInfoFlags.m_fields.m_largeScaleTile) && !m_av1Pipeline->TileBasedDecodingInuse()
            && !m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            if (m_isFirstTileInPartialFrm)
            {
                m_batchBuf = m_secondLevelBBArray->Fetch();

                if (m_batchBuf != nullptr)
                {
                    ResourceAutoLock resLock(m_allocator, &m_batchBuf->OsResource);
                    uint8_t *batchBufBase = (uint8_t *)resLock.LockResourceForWrite();
                    DECODE_CHK_NULL(batchBufBase);
                    DECODE_CHK_STATUS(InitPicLevelCmdBuffer(*m_batchBuf, batchBufBase));
                    HalOcaInterface::OnSubLevelBBStart(cmdBuffer, (MOS_CONTEXT_HANDLE)m_osInterface->pOsContext, &m_batchBuf->OsResource, 0, true, 0);
                    m_picCmdBuffer.cmdBuf1stLvl = &cmdBuffer;
                    DECODE_CHK_STATUS(m_picturePkt->Execute(m_picCmdBuffer));
                    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&m_picCmdBuffer, nullptr));
                }
            }

            DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferStartCmd(&cmdBuffer, m_batchBuf));
        }
        else
        {
            DECODE_CHK_STATUS(m_picturePkt->Execute(cmdBuffer));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktG12::InitDummyWL(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        const uint32_t section1[]=
        {
            0x718a0001, 0x00000001, 0x00000000, 0x68000100,
            0x71800004, 0x00000040, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x68000100, 0x71810003,
            0x0000007f, 0x20000020, 0x00000000, 0x00000000
        };

        const uint32_t section2[]=
        {
            0x71b00031, 0x000f000f, 0x00391601, 0x70080053,
            0x00950000, 0x00000000, 0x00000000, 0x00000004,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x76543210, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x7192000d,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x00000000, 0x71b20002, 0x00000000,
            0xfff00000, 0x00000000, 0x71b3000d, 0x022f4cae,
            0x00000001, 0x7f7f007f, 0x00000000, 0xd0000c02,
            0x00000000, 0x000000b0, 0x00000000, 0x0008000f,
            0x00000110, 0x00000000, 0x00000000, 0x00000000,
            0x00000000, 0x71950004, 0x00000000, 0x00000000,
            0x00000000, 0xbe000000, 0x00000001, 0x71a00001,
            0x00000050, 0x0000003d, 0x718a0001, 0x00000000,
            0x00000004, 0x77800000, 0x00100030
        };
        DECODE_CHK_NULL(m_osInterface);
        m_osInterface->pfnAddCommand(&cmdBuffer, section1, sizeof(section1));

        DECODE_CHK_STATUS(m_picturePkt->UpdatePipeBufAddrForDummyWL(cmdBuffer));
        DECODE_CHK_STATUS(m_picturePkt->UpdateIndObjAddrForDummyWL(cmdBuffer));

        m_osInterface->pfnAddCommand(&cmdBuffer, section2, sizeof(section2));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktG12::PackTileLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

        int16_t tileIdx = m_av1BasicFeature->m_tileCoding.m_curTile;

        if (tileIdx < int16_t(m_av1BasicFeature->m_tileCoding.m_totalTileNum))
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

        // For film grain frame, apply noise packet should update report global count
        bool isLastTileInFullFrm = (tileIdx == int16_t(m_av1BasicFeature->m_tileCoding.m_totalTileNum) - 1) ? 1 : 0;
        if (isLastTileInFullFrm && !m_av1BasicFeature->m_filmGrainEnabled)
        {
            DECODE_CHK_STATUS(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));
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

        if ((m_isLastTileInPartialFrm || m_av1Pipeline->TileBasedDecodingInuse())
            && !m_osInterface->pfnIsMismatchOrderProgrammingSupported())
        {
            DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
        }

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktG12::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        // Send MI_FLUSH command
        MHW_MI_FLUSH_DW_PARAMS flushDwParams;
        MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
        DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

        return MOS_STATUS_SUCCESS;
    }
}

