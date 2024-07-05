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
//! \file     decode_av1_status_report_xe2_lpm_base.h
//! \brief    Defines the class for av1 decode status report
//! \details
//!

#ifndef __DECODE_AV1_STATUS_REPORT_XE2_LPM_BASE_H__
#define __DECODE_AV1_STATUS_REPORT_XE2_LPM_BASE_H__

#include "decode_status_report.h"

namespace decode {

class DecodeAv1StatusReportXe2_Lpm_Base : public DecodeStatusReport
{
    public:
        DecodeAv1StatusReportXe2_Lpm_Base(DecodeAllocator *alloc, bool enableRcs, PMOS_INTERFACE osInterface);
        virtual ~DecodeAv1StatusReportXe2_Lpm_Base() {}

    protected:
        //!
        //! \brief  Update the status result of current report.
        //! \param  [in] statusReportData
        //!         The pointer to DecodeStatusReportData.
        //! \param  [in] decodeStatus
        //!         The RCS status report buffer.
        //! \param  [in] completed
        //!         Whether the request frame completed.
        //! \return MOS_STATUS
        //!         MOS_STATUS_SUCCESS if success, else fail reason
        //!
        virtual MOS_STATUS UpdateCodecStatus(
            DecodeStatusReportData* statusReportData,
            DecodeStatusMfx* decodeStatus,
            bool completed) override;

protected:
    PMOS_INTERFACE m_osInterface = nullptr;

MEDIA_CLASS_DEFINE_END(decode__DecodeAv1StatusReportXe2_Lpm_Base)
};
}

#endif // !__DECODE_AV1_STATUS_REPORT_XE2_LPM_BASE_H__
