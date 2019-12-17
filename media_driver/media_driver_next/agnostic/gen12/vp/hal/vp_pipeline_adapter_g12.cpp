/*
* Copyright (c) 2018, Intel Corporation
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
#include "vp_pipeline_adapter_g12.h"

VpPipelineG12Adapter::VpPipelineG12Adapter(
    PMOS_INTERFACE pOsInterface,
    PMOS_CONTEXT   pOsDriverContext,
    MOS_STATUS    *peStatus) :
    VphalStateG12Tgllp( pOsInterface, pOsDriverContext, peStatus)
{
    if (m_reporting == nullptr)
    {
       m_reporting = MOS_New(VphalFeatureReport);
    }
    if (m_reporting)
    {
        *peStatus = MOS_STATUS_SUCCESS;
    }
    else
    {
       *peStatus = MOS_STATUS_NO_SPACE;
    }
}

MOS_STATUS VpPipelineG12Adapter::Allocate(
  const VphalSettings     *pVpHalSettings)
{
    VP_FUNC_CALL();

    m_vpPipeline = std::make_shared<vp::VpPipelineG12>(m_osInterface, m_reporting);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpPipeline);

    MOS_ZeroMemory(&m_vpMhwinterface, sizeof(VP_MHWINTERFACE));

    m_vpMhwinterface.m_platform = m_platform;
    m_vpMhwinterface.m_waTable  = m_waTable;
    m_vpMhwinterface.m_skuTable = m_skuTable;

    m_vpMhwinterface.m_osInterface      = m_osInterface;
    m_vpMhwinterface.m_renderHal        = m_renderHal;
    m_vpMhwinterface.m_veboxInterface   = m_veboxInterface;
    m_vpMhwinterface.m_sfcInterface     = m_sfcInterface;
    m_vpMhwinterface.m_renderer         = m_renderer;
    m_vpMhwinterface.m_cpInterface      = m_cpInterface;
    m_vpMhwinterface.m_mhwMiInterface   = m_renderHal->pMhwMiInterface;
    m_vpMhwinterface.m_statusTable      = &m_statusTable;

    if (m_veboxInterface &&
        m_veboxInterface->m_veboxSettings.uiNumInstances > 0 &&
        m_veboxInterface->m_veboxHeap == nullptr)
    {
        // Allocate VEBOX Heap
        VP_PUBLIC_CHK_STATUS_RETURN(m_veboxInterface->CreateHeap());
    }

    // Set VP pipeline mhw interfaces
    m_vpPipeline->SetVpPipelineMhwInterfce(&m_vpMhwinterface);

    VphalState::Allocate(pVpHalSettings);

    return m_vpPipeline->Init((void*)pVpHalSettings);
}

MOS_STATUS VpPipelineG12Adapter::Execute(void * params)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    VP_FUNC_CALL();

    eStatus = m_vpPipeline->Prepare(params);
    if (eStatus != MOS_STATUS_SUCCESS)
    {
        if (eStatus == MOS_STATUS_UNIMPLEMENTED)
        {
            VP_PUBLIC_NORMALMESSAGE("Features are UNIMPLEMENTED on APG now \n");
            return eStatus;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(eStatus);
        }
    }

    return m_vpPipeline->Execute();
}

MOS_STATUS VpPipelineG12Adapter::GetStatusReport(PQUERY_STATUS_REPORT_APP pQueryReport, uint16_t numStatus)
{
    VP_FUNC_CALL();

    return VphalState::GetStatusReport(pQueryReport, numStatus);
}

void VpPipelineG12Adapter::Destroy()
{
    VP_FUNC_CALL();

    m_vpPipeline->Destroy();
    m_vpPipeline = nullptr;
    MOS_Delete(m_reporting);
}

MOS_STATUS VpPipelineG12Adapter::Render(PCVPHAL_RENDER_PARAMS pcRenderParams)
{
    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;

    if ((m_vpMhwinterface.m_osInterface != nullptr) && (pcRenderParams != nullptr))
    {
        // Set the component info
        m_vpMhwinterface.m_osInterface->Component = pcRenderParams->Component;

        // Init component(DDI entry point) info for perf measurement
        m_vpMhwinterface.m_osInterface->pfnSetPerfTag(m_vpMhwinterface.m_osInterface, VPHAL_NONE);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(Prepare(pcRenderParams));

    void *params = (void *)&m_vpPipelineParams;

    eStatus = Execute(params);

    if (eStatus == MOS_STATUS_SUCCESS)
    {
        m_bApgEnabled = true;
        VP_PUBLIC_NORMALMESSAGE("APG Execution successfully, return \n");
        return eStatus;
    }
    else
    {
        m_bApgEnabled = false;
        return VphalState::Render(pcRenderParams);
    }
}

MOS_STATUS VpPipelineG12Adapter::Prepare(PCVPHAL_RENDER_PARAMS pcRenderParams)
{
    VP_PUBLIC_CHK_NULL_RETURN(pcRenderParams);

    if (m_vpPipeline)
    {
        m_vpPipelineParams = *(PVP_PIPELINE_PARAMS)pcRenderParams;

        // default render of video
        m_vpPipelineParams.bIsDefaultStream = true;
    }

    return MOS_STATUS_SUCCESS;
}

VphalFeatureReport * VpPipelineG12Adapter::GetRenderFeatureReport()
{
    if (m_bApgEnabled)
    {
        return m_reporting;
    }
    else
    {
        return VphalState::GetRenderFeatureReport();
    }
}
