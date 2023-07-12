/*
* Copyright (c) 2019, Intel Corporation
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
//! \file     encodecp.cpp
//! \brief    Defines the common interface for secure encode
//!
#include "encodecp.h"
#include "encode_utils.h"
#include "encode_status_report.h"
#include "codechal_setting.h"
#include "mos_os_cp_interface_specific.h"  

namespace encode {

    bool EncodeCp::isCpEnabled()
    {
        return m_cpEnabled;
    }

    MOS_STATUS EncodeCp::setStatusReport(MediaStatusReport *statusReport)
    {
        m_statusReport = statusReport;
        return MOS_STATUS_SUCCESS;
    }
    MOS_STATUS EncodeCp::RegisterParams(void *settings)
    {
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(settings);
        ENCODE_CHK_NULL_RETURN(m_cpInterface);
        m_cpInterface->RegisterParams(((CodechalSetting *)settings)->GetCpParams());
        m_cpEnabled = m_osInterface->osCpInterface->IsCpEnabled();
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeCp::UpdateParams(bool input)
    {
        if (!m_cpEnabled)
        {
            return MOS_STATUS_SUCCESS;
        }
        ENCODE_FUNC_CALL();
        ENCODE_CHK_NULL_RETURN(m_cpInterface);
        ENCODE_CHK_STATUS_RETURN(m_cpInterface->UpdateParams(input));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeCp::StartCpStatusReport(MOS_COMMAND_BUFFER *cmdBuffer)
    {
        if (!m_cpEnabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        PMOS_RESOURCE m_hwcounterBuf = (static_cast<EncoderStatusReport *>(m_statusReport))->GetHwCtrBuf(); 
        ENCODE_CHK_NULL_RETURN(m_hwcounterBuf);
        ENCODE_CHK_NULL_RETURN(m_cpInterface);

        uint32_t offset      = m_statusReport->GetIndex(m_statusReport->GetSubmittedCount());

        ENCODE_CHK_STATUS_RETURN(m_cpInterface->ReadEncodeCounterFromHW(
            m_osInterface,
            cmdBuffer,
            m_hwcounterBuf,
            (uint16_t)offset));
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeCp::UpdateCpStatusReport(void *pStatusReportData)
    {
        if (!m_cpEnabled)
        {
            return MOS_STATUS_SUCCESS;
        }

        ENCODE_CHK_NULL_RETURN(pStatusReportData);
        EncodeStatusReportData *statusReportData = static_cast<EncodeStatusReportData*>(pStatusReportData);
        uint64_t *              hwcounter        = statusReportData->hwCtr;
        ENCODE_CHK_NULL_RETURN(hwcounter);

        statusReportData->hwCounterValue.Count = *hwcounter;
        //Report back in Big endian
        statusReportData->hwCounterValue.Count = SwapEndianness(statusReportData->hwCounterValue.Count);
        //IV value computation
        statusReportData->hwCounterValue.IV = *(++hwcounter);
        statusReportData->hwCounterValue.IV = SwapEndianness(statusReportData->hwCounterValue.IV);
        ENCODE_NORMALMESSAGE(
            "encodeStatusReport->HWCounterValue.Count = 0x%llx, encodeStatusReport->HWCounterValue.IV = 0x%llx",
            statusReportData->hwCounterValue.Count,
            statusReportData->hwCounterValue.IV);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS EncodeCp::patchForHM()
    {
        if (m_cpEnabled && m_osInterface && m_osInterface->osCpInterface)
        {
            return m_osInterface->osCpInterface->PermeatePatchForHM(nullptr, nullptr, nullptr);
        }
        return MOS_STATUS_SUCCESS;
    }
}