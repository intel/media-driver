/*
* Copyright (c) 2019-2020, Intel Corporation
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
#include "decode_av1_pipeline.h"
#include "decode_av1_basic_feature.h"
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

        DECODE_CHK_NULL(cmdBuffer);
        DECODE_CHK_NULL(m_hwInterface);

        DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_av1BasicFeature->m_destSurface));

        SetPerfTag(CODECHAL_DECODE_MODE_AV1VLD, m_av1BasicFeature->m_pictureCodingType);

        auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);
        HalOcaInterface::On1stLevelBBStart(*cmdBuffer, *m_osInterface->pOsContext, m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);

        DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));
        DECODE_CHK_STATUS(PackTileLevelCmds(*cmdBuffer));

        HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);

        m_av1BasicFeature->m_tileCoding.m_curTile++; //Update tile index of current frame

        DECODE_CHK_STATUS(m_allocator->SyncOnResource(&m_av1BasicFeature->m_resDataBuffer, false));

        //Set ReadyToExecute to true for the last tile of the frame
        Mos_Solo_SetReadyToExecute(m_osInterface, m_av1BasicFeature->frameCompletedFlag);

        DECODE_CHK_STATUS(Mos_Solo_PostProcessDecode(m_osInterface, &m_av1BasicFeature->m_destSurface));

        if(m_av1BasicFeature->frameCompletedFlag && !m_av1BasicFeature->m_filmGrainEnabled)
        {
            m_osInterface->pfnIncPerfFrameID(m_osInterface);
            m_osInterface->pfnResetPerfBufferID(m_osInterface);
        }

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

        if (IsPrologRequired())
        {
            DECODE_CHK_STATUS(AddForceWakeup(cmdBuffer));
            DECODE_CHK_STATUS(SendPrologWithFrameTracking(cmdBuffer, true));
        }

        DECODE_CHK_STATUS(StartStatusReport(statusReportMfx, &cmdBuffer));
        DECODE_CHK_STATUS(m_picturePkt->Execute(cmdBuffer));

        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS Av1DecodePktG12::PackTileLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
    {
        DECODE_FUNC_CALL();

        int16_t tileIdx = m_av1BasicFeature->m_tileCoding.m_curTile;

        if ( tileIdx < int16_t(m_av1BasicFeature->m_tileCoding.m_numTiles))
        {
            DECODE_CHK_STATUS(m_tilePkt->Execute(cmdBuffer, tileIdx));
        }

        DECODE_CHK_STATUS(VdMemoryFlush(cmdBuffer));
        DECODE_CHK_STATUS(VdPipelineFlush(cmdBuffer));
        DECODE_CHK_STATUS(EnsureAllCommandsExecuted(cmdBuffer));
        DECODE_CHK_STATUS(EndStatusReport(statusReportMfx, &cmdBuffer));

        bool isLastTileInFrm = (tileIdx == int16_t(m_av1BasicFeature->m_tileCoding.m_numTiles) - 1) ? 1 : 0;

        // For film grain frame, apply noise packet should update report global count
        if (isLastTileInFrm && !m_av1BasicFeature->m_filmGrainEnabled)
        {
            DECODE_CHK_STATUS(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));
        }

        CODECHAL_DEBUG_TOOL(
            if (m_mmcState) {
                m_mmcState->UpdateUserFeatureKey(&(m_av1BasicFeature->m_destSurface));
            })

        if (isLastTileInFrm || !m_av1Pipeline->FrameBasedDecodingInUse())
        {
            if (MEDIA_IS_WA(m_av1Pipeline->GetWaTable(), WaRSDisableMediaPGForAV1CodecWL))
            {
                cmdBuffer.Attributes.bDisablePowerGating = true;
            }
            else
            {
                cmdBuffer.Attributes.bDisablePowerGating = false;
            }

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

