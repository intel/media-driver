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
#include "vp_pipeline_adapter_xe3_lpm.h"
#include "vp_platform_interface.h"
#include "vp_common_cache_settings.h"

VpPipelineAdapterXe3_Lpm::VpPipelineAdapterXe3_Lpm(
    PMOS_INTERFACE              pOsInterface,
    vp::VpPlatformInterface     &vpPlatformInterface,
    MOS_STATUS                  &eStatus) :
    VpPipelineAdapter(vpPlatformInterface, eStatus)
{
    if (MOS_FAILED(eStatus))
    {
        MOS_OS_ASSERTMESSAGE("VpPipelineAdapterXe3_Lpm construct failed due to base class returned failure: eStatus = %d.", eStatus);
        return;
    }
}

VpPipelineAdapterXe3_Lpm::~VpPipelineAdapterXe3_Lpm()
{
}

MOS_STATUS VpPipelineAdapterXe3_Lpm::Render(PCVPHAL_RENDER_PARAMS pcRenderParams)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = VpPipelineAdapter::Render(pcRenderParams);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        return eStatus;
    }
    else
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
}

MOS_STATUS VpPipelineAdapterXe3_Lpm::Allocate(
    const VpSettings *pVpHalSettings)
{
    VP_FUNC_CALL();

    VP_MHWINTERFACE vpMhwinterface = {};  //!< vp Mhw Interface
    MOS_STATUS      status         = VpPipelineAdapter::GetVpMhwInterface(vpMhwinterface);
    if (MOS_FAILED(status))
    {
        return status;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(RegisterCacheSettings());

    return Init(pVpHalSettings, vpMhwinterface);
}

MOS_STATUS VpPipelineAdapterXe3_Lpm::GetStatusReport(
    PQUERY_STATUS_REPORT_APP  pQueryReport,
    uint16_t                  numStatus)
{
    VP_FUNC_CALL();

    return VpPipelineAdapterBase::GetStatusReport(pQueryReport, numStatus);
}

VphalFeatureReport *VpPipelineAdapterXe3_Lpm::GetRenderFeatureReport()
{
    VP_FUNC_CALL();

    if (m_bApgEnabled)
    {
        return m_vpPipeline == nullptr ? nullptr : m_vpPipeline->GetFeatureReport();
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS VpPipelineAdapterXe3_Lpm::Execute(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();

    return VpPipelineAdapter::Execute(params, m_vprenderHal);
}

MOS_STATUS VpPipelineAdapterXe3_Lpm::RegisterCacheSettings()
{
    VP_FUNC_CALL();
    bool res = m_osInterface->pfnInsertCacheSetting(CACHE_COMPONENT_VP, &g_vp_cacheSettings);

    if (res == false)
    {
        VP_PUBLIC_ASSERTMESSAGE("CacheTables insert failed");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }
    return MOS_STATUS_SUCCESS;
}