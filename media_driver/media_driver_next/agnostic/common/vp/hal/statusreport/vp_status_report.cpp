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
//! \file     vp_status_report.cpp
//! \brief    Defines the interface for vp status report
//! \details  vp status will allocate and destory buffers, the caller
//!           can use directly
//!

#include "vp_status_report.h"

namespace vp
{
VPStatusReport::VPStatusReport(PMOS_INTERFACE  pOsInterface) :
    m_osInterface(pOsInterface)
{
    MOS_ZeroMemory(&m_StatusTableUpdateParams, sizeof(m_StatusTableUpdateParams));
}

void VPStatusReport::SetPipeStatusReportParams(PVP_PIPELINE_PARAMS pVpParams, PVPHAL_STATUS_TABLE pStatusTable)
{
    bool bSurfIsRenderTarget                      = (pVpParams->pTarget[0]->SurfType == SURF_OUT_RENDERTARGET);
    m_StatusTableUpdateParams.bReportStatus       = (pVpParams->bReportStatus);
    m_StatusTableUpdateParams.bSurfIsRenderTarget = bSurfIsRenderTarget;
    m_StatusTableUpdateParams.pStatusTable        = pStatusTable;
    m_StatusTableUpdateParams.StatusFeedBackID    = pVpParams->StatusFeedBackID;
}

MOS_STATUS VPStatusReport::UpdateStatusTableAfterSubmit(
    MOS_STATUS                  eLastStatus)
{

    PVPHAL_STATUS_ENTRY             pStatusEntry;
    bool                            bEmptyTable;
    MOS_STATUS                      eStatus;
    uint32_t                        dwLastTag;
    PVPHAL_STATUS_TABLE             pStatusTable;
    uint32_t                        dwStatusFeedBackID;

    eStatus = MOS_STATUS_SUCCESS;

    MOS_GPU_CONTEXT eMosGpuContext = m_osInterface->pfnGetGpuContext(m_osInterface);

    if (!m_StatusTableUpdateParams.bReportStatus ||
        !m_StatusTableUpdateParams.bSurfIsRenderTarget)
    {
        goto finish;
    }

    VPHAL_PUBLIC_CHK_NULL(m_StatusTableUpdateParams.pStatusTable);
    VPHAL_PUBLIC_CHK_NULL(m_osInterface);

    pStatusTable       = m_StatusTableUpdateParams.pStatusTable;
    dwStatusFeedBackID = m_StatusTableUpdateParams.StatusFeedBackID;

    VPHAL_PUBLIC_ASSERT(pStatusTable->uiHead < VPHAL_STATUS_TABLE_MAX_SIZE);
    VPHAL_PUBLIC_ASSERT(pStatusTable->uiCurrent < VPHAL_STATUS_TABLE_MAX_SIZE);

    bEmptyTable = (pStatusTable->uiCurrent == pStatusTable->uiHead);
    if (!bEmptyTable)
    {
        uint32_t uiLast                 = (pStatusTable->uiCurrent - 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);
        bool bSameFrameIdWithLastRender = (pStatusTable->aTableEntries[uiLast].StatusFeedBackID == dwStatusFeedBackID);
        if (bSameFrameIdWithLastRender)
        {
            pStatusTable->uiCurrent = uiLast;
        }
    }

    pStatusEntry                    = &pStatusTable->aTableEntries[pStatusTable->uiCurrent];
    pStatusEntry->StatusFeedBackID  = dwStatusFeedBackID;
    pStatusEntry->GpuContextOrdinal = eMosGpuContext;
    dwLastTag                       = m_osInterface->pfnGetGpuStatusTag(m_osInterface, eMosGpuContext) - 1;
    pStatusEntry->dwTag             = dwLastTag;
    pStatusEntry->dwStatus          = (eLastStatus == MOS_STATUS_SUCCESS)? VPREP_NOTREADY : VPREP_ERROR;
    pStatusTable->uiCurrent         = (pStatusTable->uiCurrent + 1) & (VPHAL_STATUS_TABLE_MAX_SIZE - 1);

finish:
    return eStatus;
}
}  // namespace vp
