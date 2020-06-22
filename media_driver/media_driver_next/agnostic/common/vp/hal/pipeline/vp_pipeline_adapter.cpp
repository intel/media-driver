/*
* Copyright (c) 2020, Intel Corporation
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
#include "vp_pipeline_adapter.h"
#include "vp_platform_interface.h"

VpPipelineAdapter::VpPipelineAdapter(
    vp::VpPlatformInterface     &vpPlatformInterface,
    MOS_STATUS                  &eStatus) :
    m_vpPlatformInterface(vpPlatformInterface)
{
    if (MOS_FAILED(eStatus))
    {
        MOS_OS_ASSERTMESSAGE("VpPipelineAdapter construct failed due to VphalStateG12Tgllp() returned failure: eStatus = %d.", eStatus);
        return;
    }
}

//!
//! \brief    VpPipelineG12Adapter Destuctor
//! \details  Destroys VpPipelineG12Adapter and all internal states and objects
//! \return   void
//!
VpPipelineAdapter::~VpPipelineAdapter()
{
    Destroy();
    vp::VpPlatformInterface *pIntf = &m_vpPlatformInterface;
    MOS_Delete(pIntf);
};

MOS_STATUS VpPipelineAdapter::Init(
  const VphalSettings     *pVpHalSettings, VphalState &vphalState)
{
    VP_FUNC_CALL();

    if (m_reporting == nullptr)
    {
        m_reporting = MOS_New(VphalFeatureReport);
    }
    VP_PUBLIC_CHK_NULL_RETURN(m_reporting);

    m_vpPipeline = std::make_shared<vp::VpPipeline>(vphalState.GetOsInterface(), m_reporting);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpPipeline);
    VP_PUBLIC_CHK_NULL_RETURN(vphalState.GetRenderHal());

    MOS_ZeroMemory(&m_vpMhwinterface, sizeof(VP_MHWINTERFACE));

    m_vpMhwinterface.m_platform = vphalState.GetPlatform();
    m_vpMhwinterface.m_waTable  = vphalState.GetWaTable();
    m_vpMhwinterface.m_skuTable = vphalState.GetSkuTable();

    m_vpMhwinterface.m_osInterface      = vphalState.GetOsInterface();
    m_vpMhwinterface.m_renderHal        = vphalState.GetRenderHal();
    m_vpMhwinterface.m_veboxInterface   = vphalState.GetVeboxInterface();
    m_vpMhwinterface.m_sfcInterface     = vphalState.GetSfcInterface();
    m_vpMhwinterface.m_renderer         = vphalState.GetRenderer();
    m_vpMhwinterface.m_cpInterface      = vphalState.GetCpInterface();
    m_vpMhwinterface.m_mhwMiInterface   = vphalState.GetRenderHal()->pMhwMiInterface;
    m_vpMhwinterface.m_statusTable      = &vphalState.GetStatusTable();
    m_vpMhwinterface.m_vpPlatformInterface = &m_vpPlatformInterface;

    if (vphalState.GetVeboxInterface() &&
        vphalState.GetVeboxInterface()->m_veboxSettings.uiNumInstances > 0 &&
        vphalState.GetVeboxInterface()->m_veboxHeap == nullptr)
    {
        // Allocate VEBOX Heap
        VP_PUBLIC_CHK_STATUS_RETURN(vphalState.GetVeboxInterface()->CreateHeap());
    }

    // Set VP pipeline mhw interfaces
    m_vpPipeline->SetVpPipelineMhwInterfce(&m_vpMhwinterface);

    return m_vpPipeline->Init((void*)pVpHalSettings);
}

MOS_STATUS VpPipelineAdapter::Execute(void * params)
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

void VpPipelineAdapter::Destroy()
{
    VP_FUNC_CALL();
    if (m_vpPipeline)
    {
        m_vpPipeline->Destroy();
        m_vpPipeline = nullptr;
    }
    MOS_Delete(m_reporting);
}

MOS_STATUS VpPipelineAdapter::Render(PCVPHAL_RENDER_PARAMS pcRenderParams)
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
        return eStatus;
    }
}

MOS_STATUS VpPipelineAdapter::Prepare(PCVPHAL_RENDER_PARAMS pcRenderParams)
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
