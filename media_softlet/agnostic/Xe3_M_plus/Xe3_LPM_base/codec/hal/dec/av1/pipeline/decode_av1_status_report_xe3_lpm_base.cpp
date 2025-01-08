/*
* Copyright (c) 2022, Intel Corporation
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
//! \file     decode_av1_status_report_xe3_lpm_base.cpp
//! \brief    Defines the common interface for av1 decode status reporter
//! \details
//!

#include "decode_av1_status_report_xe3_lpm_base.h"

namespace decode {

    DecodeAv1StatusReportXe3_Lpm_Base::DecodeAv1StatusReportXe3_Lpm_Base(
        DecodeAllocator* allocator, bool enableRcs, PMOS_INTERFACE osInterface):
        DecodeStatusReport(allocator, enableRcs)
    {
        DECODE_FUNC_CALL()

        m_osInterface = osInterface;
    }

    MOS_STATUS DecodeAv1StatusReportXe3_Lpm_Base::UpdateCodecStatus(
        DecodeStatusReportData* statusReportData,
        DecodeStatusMfx* decodeStatus,
        bool completed)
    {
        DECODE_FUNC_CALL()

        DECODE_CHK_NULL(statusReportData);
        DECODE_CHK_NULL(decodeStatus);

        if(!completed)
        {
            statusReportData->codecStatus = CODECHAL_STATUS_INCOMPLETE;
        }
        else if (decodeStatus->m_mmioErrorStatusReg != 0)
        {
            // From HW perspective, expontential golomb error and exit symbol error are real errors
            // not report exit symbol error now because of HW issue.
            if ((decodeStatus->m_mmioErrorStatusReg & 0x40000000) != 0)
            {
                statusReportData->codecStatus = CODECHAL_STATUS_ERROR;
            }
            else
            {
                statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;
            }

            DECODE_ASSERTMESSAGE(
                "Superblock Y Position for the First Error detected in the current decoded tile: %d, \n"
                "Superblock X Position for the First Error detected in the current decoded tile: %d, \n"
                "Bitstream Overflow:            %d, \n"
                "Bitstream Underflow:           %d, \n"
                "Expontential Golomb Error:     %d, \n"
                "Exit Symbol Error:             %d",
                decodeStatus->m_mmioErrorStatusReg & 0x7ff,
                (decodeStatus->m_mmioErrorStatusReg & 0x7f0000) >> 0x10,
                (decodeStatus->m_mmioErrorStatusReg & 0x10000000) != 0 ? true : false,
                (decodeStatus->m_mmioErrorStatusReg & 0x20000000) != 0 ? true : false,
                (decodeStatus->m_mmioErrorStatusReg & 0x40000000) != 0 ? true : false,
                (decodeStatus->m_mmioErrorStatusReg & 0x80000000) != 0 ? true : false);

#if (_DEBUG || _RELEASE_INTERNAL)
            if (m_osInterface)
            {
                ReportUserSettingForDebug(
                    m_osInterface->pfnGetUserSettingInstance(m_osInterface),
                    "Av1 Error Status Addr Value",
                    decodeStatus->m_mmioErrorStatusReg,
                    MediaUserSetting::Group::Sequence);
            }
#endif
        }
        else
        {
            statusReportData->codecStatus = CODECHAL_STATUS_SUCCESSFUL;
        }

        return MOS_STATUS_SUCCESS;
    }
}
