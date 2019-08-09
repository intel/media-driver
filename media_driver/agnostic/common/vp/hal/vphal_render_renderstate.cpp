/*
* Copyright (c) 2016-2017, Intel Corporation
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
//! \file     vphal_render_renderstate.cpp
//! \brief    VPHAL RenderState class implementation
//! \details  RenderState class function implementation
//!
#include "vphal_renderer.h"
#include "vphal_render_renderstate.h"

//!
//! \brief    RenderState Constructor
//! \details  Construct RenderState and allocate member data structure
//! \param    [in] pOsInterface
//!           Pointer to MOS interface structure
//! \param    [in] pRenderHal
//!           Pointer to RenderHal interface structure
//! \param    [in] pPerfData
//!           Pointer to performance data structure
//! \param    [out] peStatus
//!           Pointer to MOS status
//!
RenderState::RenderState(
    PMOS_INTERFACE              pOsInterface,
    PRENDERHAL_INTERFACE        pRenderHal,
    PVPHAL_RNDR_PERF_DATA       pPerfData,
    MOS_STATUS                  *peStatus) :
    m_pOsInterface(pOsInterface),
    m_pRenderHal(pRenderHal),
    m_pSkuTable(nullptr),
    m_pWaTable(nullptr),
    m_bDisableRender(false),
    m_bSingleSlice(false),
    m_pPerfData(pPerfData),
    m_reporting(nullptr)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    VPHAL_RENDER_CHK_NULL(pRenderHal);

    // Connect renderer to other VPHAL components (HW/OS interfaces)
    m_pWaTable  = pRenderHal->pWaTable;
    m_pSkuTable = pRenderHal->pSkuTable;

finish:
    if (peStatus)
    {
        *peStatus = eStatus;
    }
}

//!
//! \brief    Render function for the pass
//! \details  The render function coordinates the advanced renderers and basic
//!           renders in one pass
//! \param    [in,out] pRenderer
//!           VPHAL renderer pointer
//! \param    [in,out] pRenderParams
//!           Pointer to VPHAL render parameter
//! \return   MOS_STATUS
//!           Return MOS_STATUS_SUCCESS if successful, otherwise failed
//!
void RenderState::SetStatusReportParams(
    VphalRenderer           *pRenderer,
    PVPHAL_RENDER_PARAMS    pRenderParams)
{
    bool bSurfIsRenderTarget = (pRenderParams->pTarget[0]->SurfType == SURF_OUT_RENDERTARGET);
    m_StatusTableUpdateParams.bReportStatus       = (pRenderParams->bReportStatus);
    m_StatusTableUpdateParams.bSurfIsRenderTarget = bSurfIsRenderTarget;
    m_StatusTableUpdateParams.pStatusTable        = pRenderer->m_statusTable;
    m_StatusTableUpdateParams.StatusFeedBackID    = pRenderParams->StatusFeedBackID;
}
