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
#include "vp_pipeline_adapter_legacy.h"
#include "vp_platform_interface.h"
#include "vp_debug.h"
#include "vp_user_feature_control.h"

VpPipelineAdapterLegacy::VpPipelineAdapterLegacy(
    vp::VpPlatformInterface     &vpPlatformInterface,
    MOS_STATUS                  &eStatus): 
    m_vpPlatformInterface(vpPlatformInterface)
{
    if (MOS_FAILED(eStatus))
    {
        MOS_OS_ASSERTMESSAGE("VpPipelineAdapter construct failed due to VphalStateG12Tgllp() returned failure: eStatus = %d.", eStatus);
        return;
    }
}

//!
//! \brief    VpPipelineAdapter Destuctor
//! \details  Destroys VpPipelineAdapter and all internal states and objects
//! \return   void
//!
VpPipelineAdapterLegacy::~VpPipelineAdapterLegacy()
{
    Destroy();
};

MOS_STATUS VpPipelineAdapterLegacy::Init(
    const VphalSettings *pVpHalSettings, VP_MHWINTERFACE vpMhwinterface)
{
    VP_FUNC_CALL();

    m_vpPipeline = std::make_shared<vp::VpPipeline>(vpMhwinterface.m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpPipeline);
    VP_PUBLIC_CHK_NULL_RETURN(vpMhwinterface.m_renderHal);

    vpMhwinterface.m_vpPlatformInterface = &m_vpPlatformInterface;

    // Init Video processing settings
    VP_SETTINGS settings;

    MOS_ZeroMemory(&settings, sizeof(VP_SETTINGS));
    settings.disableHdr             = pVpHalSettings->disableHdr;
    settings.disableDnDi            = pVpHalSettings->disableDnDi;
    settings.kernelUpdate           = pVpHalSettings->kernelUpdate;
    settings.veboxParallelExecution = pVpHalSettings->veboxParallelExecution;

    vpMhwinterface.m_settings         = (void *) &settings;

    if (vpMhwinterface.m_veboxInterface)
    {
        m_veboxItf = std::static_pointer_cast<mhw::vebox::Itf>(vpMhwinterface.m_veboxInterface->GetNewVeboxInterface());
    }

    if (m_veboxItf)
    {
        const MHW_VEBOX_HEAP* veboxHeap = nullptr;
        m_veboxItf->GetVeboxHeapInfo(&veboxHeap);
        uint32_t uiNumInstances = m_veboxItf->GetVeboxNumInstances();

        if (uiNumInstances > 0 &&
            veboxHeap == nullptr)
        {
            // Allocate VEBOX Heap
            VP_PUBLIC_CHK_STATUS_RETURN(m_veboxItf->CreateHeap());
        }
    }
    else if (vpMhwinterface.m_veboxInterface &&
        vpMhwinterface.m_veboxInterface->m_veboxSettings.uiNumInstances > 0 &&
        vpMhwinterface.m_veboxInterface->m_veboxHeap == nullptr)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(vpMhwinterface.m_veboxInterface->CreateHeap());
    }

    return m_vpPipeline->Init(&vpMhwinterface);
}

MOS_STATUS VpPipelineAdapterLegacy::Execute(PVP_PIPELINE_PARAMS params, PRENDERHAL_INTERFACE renderHal)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;
    vp::VP_PARAMS vpParams = {};

    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(renderHal);

    vpParams.type = vp::PIPELINE_PARAM_TYPE_LEGACY;
    vpParams.renderParams = params;

    eStatus = m_vpPipeline->Prepare(&vpParams);
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
    VPHAL_DBG_OCA_DUMPER_SET_RENDER_PARAM(renderHal, params);

    return m_vpPipeline->Execute();
}

void VpPipelineAdapterLegacy::Destroy()
{
    VP_FUNC_CALL();
    if (m_vpPipeline)
    {
        m_vpPipeline->Destroy();
        m_vpPipeline = nullptr;
    }
    vp::VpPlatformInterface *pIntf = &m_vpPlatformInterface;
    MOS_Delete(pIntf);
}

MOS_STATUS VpPipelineAdapterLegacy::Render(PCVPHAL_RENDER_PARAMS pcRenderParams)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
    VP_PIPELINE_PARAMS params = {};

    VP_PUBLIC_CHK_NULL_RETURN(pcRenderParams);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpPipeline);

    params = *(PVP_PIPELINE_PARAMS)pcRenderParams;
    // default render of video
    params.bIsDefaultStream = true;

    eStatus = Execute(&params);

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
