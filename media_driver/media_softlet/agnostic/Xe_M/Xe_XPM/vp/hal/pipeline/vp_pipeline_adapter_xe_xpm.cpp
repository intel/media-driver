/*
* Copyright (c) 2020-2021, Intel Corporation
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
#include "vp_pipeline_adapter_xe_xpm.h"
#include "vp_platform_interface.h"

VpPipelineAdapterXe_Xpm::VpPipelineAdapterXe_Xpm(
    PMOS_INTERFACE              pOsInterface,
    vp::VpPlatformInterface     &vpPlatformInterface,
    MOS_STATUS                  &eStatus) :
    VphalStateXe_Xpm(pOsInterface, &eStatus),
    VpPipelineAdapterLegacy(vpPlatformInterface, eStatus)
{
    if (MOS_FAILED(eStatus))
    {
        MOS_OS_ASSERTMESSAGE("VpPipelineAdapterXe_Xpm construct failed due to base class returned failure: eStatus = %d.", eStatus);
        MT_ERR1(MT_VP_HAL_PIPELINE, MT_ERROR_CODE, eStatus);
        return;
    }
}

VpPipelineAdapterXe_Xpm::~VpPipelineAdapterXe_Xpm()
{
}

MOS_STATUS VpPipelineAdapterXe_Xpm::Render(PCVPHAL_RENDER_PARAMS pcRenderParams)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = VpPipelineAdapterLegacy::Render(pcRenderParams);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }
    else
    {
        return VphalStateXe_Xpm::Render(pcRenderParams);
    }
}

MOS_STATUS VpPipelineAdapterXe_Xpm::Allocate(
    const VphalSettings     *pVpHalSettings)
{
    VP_FUNC_CALL();

    MOS_STATUS status = VphalStateXe_Xpm::Allocate(pVpHalSettings);
    if (MOS_FAILED(status))
    {
        MT_ERR1(MT_VP_HAL_PIPELINE, MT_ERROR_CODE, status);
        return status;
    }

    VP_MHWINTERFACE vpMhwinterface = {};  //!< vp Mhw Interface
    status                         = VphalStateXe_Xpm::GetVpMhwInterface(vpMhwinterface);
    if (MOS_FAILED(status))
    {
        MT_ERR1(MT_VP_HAL_PIPELINE, MT_ERROR_CODE, status);
        return status;
    }
    return Init(pVpHalSettings, vpMhwinterface);
}

MOS_STATUS VpPipelineAdapterXe_Xpm::GetStatusReport(
    PQUERY_STATUS_REPORT_APP  pQueryReport,
    uint16_t                  numStatus)
{
    VP_FUNC_CALL();

    return VphalStateXe_Xpm::GetStatusReport(pQueryReport, numStatus);
}

VphalFeatureReport* VpPipelineAdapterXe_Xpm::GetRenderFeatureReport()
{
    VP_FUNC_CALL();

    if (m_bApgEnabled)
    {
        return m_vpPipeline == nullptr ? nullptr : m_vpPipeline->GetFeatureReport();
    }
    else
    {
        return VphalStateXe_Xpm::GetRenderFeatureReport();
    }
}

MOS_STATUS VpPipelineAdapterXe_Xpm::Execute(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();

    return VpPipelineAdapterLegacy::Execute(params, this->m_renderHal);
}
