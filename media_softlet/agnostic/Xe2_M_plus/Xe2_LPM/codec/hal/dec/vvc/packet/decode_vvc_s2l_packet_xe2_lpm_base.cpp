/*
* Copyright (c) 2022-2023, Intel Corporation
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
//! \file     decode_vvc_s2l_packet_xe2_lpm_base.cpp
//! \brief    Defines the implementation of VVC decode S2L packet
//!
#include "decode_vvc_s2l_packet_xe2_lpm_base.h"
#include "mhw_mi_hwcmd_xe2_lpm_base_next.h"
#include "codec_hw_xe2_lpm_base.h"
#include "decode_vvc_s2l_packet_register_xe2_lpm.h"

namespace decode
{
MOS_STATUS VvcDecodeS2LPktXe2_Lpm_Base::Execute(MOS_COMMAND_BUFFER &cmdBuffer, bool prologNeeded)
{
    DECODE_FUNC_CALL();
    PERF_UTILITY_AUTO(__FUNCTION__, PERF_DECODE, PERF_LEVEL_HAL);

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

    MOS_RESOURCE *osResource = nullptr;
    uint32_t      offset     = 0;

    DECODE_CHK_STATUS(m_statusReport->GetAddress(decode::DecodeStatusReportType::HucErrorStatus2Mask, osResource, offset));

    DECODE_CHK_STATUS(StoreHucStatusRegister(cmdBuffer));

    // Check HuC_STATUS2 bit6, if bit6 > 0 HW continue execution following cmd, otherwise it send a COND BB END cmd.
    uint32_t compareOperation = mhw::mi::xe2_lpm_base_next::Cmd::MI_CONDITIONAL_BATCH_BUFFER_END_CMD::COMPARE_OPERATION_MADGREATERTHANIDD;
    CodechalHwInterfaceXe2_Lpm_Base *hwInterface    = dynamic_cast<CodechalHwInterfaceXe2_Lpm_Base *>(m_hwInterface);
    if (hwInterface != nullptr)
    {
        DECODE_CHK_STATUS(hwInterface->SendCondBbEndCmd(osResource, offset, 0, false, false, compareOperation, &cmdBuffer));
    }

    if (!m_vvcPipeline->IsSingleTaskPhaseSupported())
    {
        DECODE_CHK_STATUS(m_miItf->AddMiBatchBufferEnd(&cmdBuffer, nullptr));
    }

    return MOS_STATUS_SUCCESS;
}

};  // namespace decode