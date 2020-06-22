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
//! \file     vp_status_report.h
//! \brief    Defines the interface for vp status report
//! \details  vp status will allocate and destory buffers, the caller
//!           can use directly
//!

#ifndef __VP_STATUS_REPORT_H__
#define __VP_STATUS_REPORT_H__

#include "vp_pipeline_common.h"
#include "media_context.h"

namespace vp
{
class VPStatusReport
{
public:

    VPStatusReport(PMOS_INTERFACE osInterface);

    ~VPStatusReport(){};

    //!
    //! \brief    set status report params
    //! \param    [in] pVpParams
    //!           pointer to vp pipeline input params
    //! \param    [in] pStatusTable
    //!           pointer to vp pipeline status table
    //! \return   void
    //!
    void SetPipeStatusReportParams(
        PVP_PIPELINE_PARAMS pVpParams,
        PVPHAL_STATUS_TABLE pStatusTable);

    //!
    //! \brief    update status report rely on command buffer sync tag
    //! \param    [in] eLastStatus
    //!           indicating last submition is successful or not
    //! \return   MOS_STATUS
    //!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
    //!
    MOS_STATUS UpdateStatusTableAfterSubmit(
        MOS_STATUS                  eLastStatus);

protected:

    STATUS_TABLE_UPDATE_PARAMS m_StatusTableUpdateParams = {};
    MOS_INTERFACE             *m_osInterface             = nullptr;
};
}  // namespace vp

#endif //__VP_STATUS_REPORT_H__