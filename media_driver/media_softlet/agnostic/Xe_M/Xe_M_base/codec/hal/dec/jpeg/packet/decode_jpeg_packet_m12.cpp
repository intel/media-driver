/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_jpeg_packet_m12.cpp
//! \brief    Defines the interface for jpeg decode packet of Gen12
//!
#include "decode_jpeg_packet_m12.h"
#include "decode_utils.h"
#include "decode_jpeg_pipeline.h"
#include "decode_jpeg_basic_feature.h"
#include "decode_status_report_defs.h"
#include "mos_solo_generic.h"
#include "decode_status_report_defs.h"
#include "decode_resource_auto_lock.h"
#include "hal_oca_interface.h"

namespace decode{

MOS_STATUS JpegDecodePktM12::Submit(
    MOS_COMMAND_BUFFER* cmdBuffer,
    uint8_t packetPhase)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_NULL(cmdBuffer);
    DECODE_CHK_NULL(m_hwInterface);

    DECODE_CHK_STATUS(m_miInterface->SetWatchdogTimerThreshold(
        m_jpegBasicFeature->m_width, m_jpegBasicFeature->m_height, false));

    DECODE_CHK_STATUS(Mos_Solo_PreProcessDecode(m_osInterface, &m_jpegBasicFeature->m_destSurface));

    if (IsPrologRequired())
    {
        DECODE_CHK_STATUS(AddForceWakeup(*cmdBuffer));
        DECODE_CHK_STATUS(SendPrologWithFrameTracking(*cmdBuffer, true));
    }

    auto mmioRegisters = m_hwInterface->GetMfxInterface()->GetMmioRegisters(MHW_VDBOX_NODE_1);
    HalOcaInterface::On1stLevelBBStart(*cmdBuffer, *m_osInterface->pOsContext,
        m_osInterface->CurrentGpuContextHandle, *m_miInterface, *mmioRegisters);
    HalOcaInterface::OnDispatch(*cmdBuffer, *m_osInterface, *m_miInterface, *m_miInterface->GetMmioRegisters());

    DECODE_CHK_STATUS(PackPictureLevelCmds(*cmdBuffer));

    HalOcaInterface::On1stLevelBBEnd(*cmdBuffer, *m_osInterface);
    DECODE_CHK_STATUS(m_allocator->SyncOnResource(&m_jpegBasicFeature->m_resDataBuffer, false));
    DECODE_CHK_STATUS(Mos_Solo_PostProcessDecode(m_osInterface, &m_jpegBasicFeature->m_destSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktM12::PackPictureLevelCmds(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

    DECODE_CHK_STATUS(StartStatusReport(statusReportMfx, &cmdBuffer));
    DECODE_CHK_STATUS(m_picturePkt->Execute(cmdBuffer));

    DECODE_CHK_STATUS(EnsureAllCommandsExecuted(cmdBuffer));
    DECODE_CHK_STATUS(EndStatusReport(statusReportMfx, &cmdBuffer));
    DECODE_CHK_STATUS(UpdateStatusReport(statusReportGlobalCount, &cmdBuffer));

    DECODE_CHK_STATUS(m_miInterface->AddMiBatchBufferEnd(&cmdBuffer, nullptr));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDecodePktM12::EnsureAllCommandsExecuted(MOS_COMMAND_BUFFER &cmdBuffer)
{
    DECODE_FUNC_CALL();

    // Send MI_FLUSH command
    MHW_MI_FLUSH_DW_PARAMS flushDwParams;
    MOS_ZeroMemory(&flushDwParams, sizeof(flushDwParams));
    DECODE_CHK_STATUS(m_miInterface->AddMiFlushDwCmd(&cmdBuffer, &flushDwParams));

    return MOS_STATUS_SUCCESS;
}

}

