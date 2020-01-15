/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     vp_pipeline.cpp
//! \brief    Defines the common interface for vp pipeline
//!           this file is for the base interface which is shared by all features.
//!

#include "vp_pipeline.h"
#include "vp_scaling_filter.h"
#include "vp_csc_filter.h"
#include "vp_rot_mir_filter.h"
#include "media_scalability_defs.h"
#include "vp_feature_manager.h"
#include "vp_packet_pipe.h"

namespace vp
{
VpPipeline::VpPipeline(PMOS_INTERFACE osInterface, VphalFeatureReport *reporting) :
    MediaPipeline(osInterface),
    m_reporting(reporting)
{
}

VpPipeline::~VpPipeline()
{
    // Delete m_pPacketPipeFactory before m_pPacketFactory, since
    // m_pPacketFactory is referenced by m_pPacketPipeFactory.
    MOS_Delete(m_pPacketPipeFactory);
    MOS_Delete(m_pPacketFactory);
    DeletePackets();
    DeleteTasks();
    // Delete m_featureManager before m_resourceManager, since
    // m_resourceManager is referenced by m_featureManager.
    MOS_Delete(m_featureManager);
    MOS_Delete(m_resourceManager);
    MOS_Delete(m_mmc);
    MOS_Delete(m_allocator);
    MOS_Delete(m_statusReport);
    if (m_mediaContext)
    {
        MOS_Delete(m_mediaContext);
        m_mediaContext = nullptr;
    }

    // Destroy surface dumper
    VPHAL_SURF_DUMP_DESTORY(m_surfaceDumper);

    // Destroy vphal parameter dump
    VPHAL_PARAMETERS_DUMPPER_DESTORY(m_parameterDumper);
}

MOS_STATUS VpPipeline::GetStatusReport(void *status, uint16_t numStatus)
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::UserFeatureReport()
{
    VP_FUNC_CALL();

    MediaPipeline::UserFeatureReport();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_currentFrameAPGEnabled)
    {
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE_ID, 1);
    }
    else
    {
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE_ID, 0);
    }
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::Initialize(void *settings)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(MediaPipeline::InitPlatform());

    m_mediaContext = MOS_New(MediaContext, scalabilityVp, m_pvpMhwInterface, m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_mediaContext);

    m_mmc = MOS_New(VPMediaMemComp, m_osInterface, m_pvpMhwInterface->m_mhwMiInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_mmc);

    m_allocator = MOS_New(VpAllocator, m_osInterface, m_mmc);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    m_statusReport = MOS_New(VPStatusReport, m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_statusReport);

    VP_PUBLIC_CHK_STATUS_RETURN(CreateFeatureManager());
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

#if (_DEBUG || _RELEASE_INTERNAL)

    // Initialize Surface Dumper
    VPHAL_SURF_DUMP_CREATE()
    VP_PUBLIC_CHK_NULL_RETURN(m_surfaceDumper);

    // Initialize Parameter Dumper
    VPHAL_PARAMETERS_DUMPPER_CREATE()
    VP_PUBLIC_CHK_NULL_RETURN(m_parameterDumper);

#endif
    m_pPacketFactory = CreatePacketFactory();
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketFactory);

    // Create active tasks
    MediaTask *pTask = GetTask(MediaTask::TaskType::cmdTask);
    VP_PUBLIC_CHK_NULL_RETURN(pTask);
    VP_PUBLIC_CHK_STATUS_RETURN(m_pPacketFactory->Initialize(pTask, m_pvpMhwInterface, m_allocator, m_mmc));

    m_pPacketPipeFactory = MOS_New(PacketPipeFactory, *m_pPacketFactory);
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketPipeFactory);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::ExecuteVpPipeline()
{
    VP_FUNC_CALL();

    MOS_STATUS      eStatus   = MOS_STATUS_SUCCESS;
    PVPHAL_RENDER_PARAMS pRenderParams = (PVPHAL_RENDER_PARAMS) m_pvpParams;
    PacketPipe *pPacketPipe = nullptr;
    VpFeatureManagerNext *featureManagerNext = nullptr;

    // Set Pipeline status Table
    m_statusReport->SetPipeStatusReportParams(m_pvpParams, m_pvpMhwInterface->m_statusTable);

    VPHAL_PARAMETERS_DUMPPER_DUMP_XML(pRenderParams);

    for (uint32_t uiLayer = 0; uiLayer < pRenderParams->uSrcCount && uiLayer < VPHAL_MAX_SOURCES; uiLayer++)
    {
        if (pRenderParams->pSrc[uiLayer])
        {
            VPHAL_SURFACE_DUMP(m_surfaceDumper,
            pRenderParams->pSrc[uiLayer],
            uiFrameCounter,
            uiLayer,
            VPHAL_DUMP_TYPE_PRE_ALL);
        }
    }

    VP_PUBLIC_CHK_NULL(m_pPacketPipeFactory);
    pPacketPipe = m_pPacketPipeFactory->CreatePacketPipe();
    VP_PUBLIC_CHK_NULL(pPacketPipe);

    featureManagerNext = dynamic_cast<VpFeatureManagerNext *>(m_featureManager);

    if (nullptr == featureManagerNext)
    {
        m_pPacketPipeFactory->ReturnPacketPipe(pPacketPipe);
        VP_PUBLIC_CHK_STATUS(MOS_STATUS_NULL_POINTER);
    }

    eStatus = featureManagerNext->InitPacketPipe(*m_pvpParams, *pPacketPipe);

    if (MOS_FAILED(eStatus))
    {
        m_pPacketPipeFactory->ReturnPacketPipe(pPacketPipe);
        VP_PUBLIC_CHK_STATUS(eStatus);
    }

    // MediaPipeline::m_statusReport is always nullptr in VP APO path right now.
    eStatus = pPacketPipe->Execute(MediaPipeline::m_statusReport, m_scalability, m_mediaContext, MOS_VE_SUPPORTED(m_osInterface), m_numVebox);

    m_pPacketPipeFactory->ReturnPacketPipe(pPacketPipe);

    VPHAL_SURFACE_PTRS_DUMP(m_surfaceDumper,
                                pRenderParams->pTarget,
                                VPHAL_MAX_TARGETS,
                                pRenderParams->uDstCount,
                                uiFrameCounter,
                                VPHAL_DUMP_TYPE_POST_ALL);

finish:

    m_statusReport->UpdateStatusTableAfterSubmit(eStatus);
    uiFrameCounter++;
    return eStatus;
}

MOS_STATUS VpPipeline::GetSystemVeboxNumber()
{
    MEDIA_SYSTEM_INFO *gtSystemInfo = m_osInterface->pfnGetGtSystemInfo(m_osInterface);

    if (gtSystemInfo != nullptr)
    {
        // Both VE mode and media solo mode should be able to get the VDBOX number via the same interface
        m_numVebox = (uint8_t)(gtSystemInfo->VEBoxInfo.NumberOfVEBoxEnabled);
    }
    else
    {
        m_numVebox = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CreateFeatureManager()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    m_resourceManager = MOS_New(VpResourceManager, *m_osInterface, *m_allocator); 
    VP_PUBLIC_CHK_NULL_RETURN(m_resourceManager);
    m_featureManager = MOS_New(VpFeatureManagerNext, *m_allocator, m_resourceManager, m_pvpMhwInterface);
    VP_PUBLIC_CHK_STATUS_RETURN(((VpFeatureManagerNext *)m_featureManager)->Initialize());

    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CheckFeatures(void *params, bool &bapgFuncSupported)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

    // Add CheckFeatures api later in FeatureManagerNext.
    VPFeatureManager paramChecker(m_pvpMhwInterface);
    return paramChecker.CheckFeatures(params, bapgFuncSupported);
}

MOS_STATUS VpPipeline::PrepareVpPipelineParams(void *params)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(params);

    m_pvpParams = (PVP_PIPELINE_PARAMS)params;

    PMOS_RESOURCE ppSource[VPHAL_MAX_SOURCES] = {nullptr};
    PMOS_RESOURCE ppTarget[VPHAL_MAX_TARGETS] = {nullptr};

    VP_PUBLIC_CHK_NULL_RETURN(m_pvpParams->pSrc[0]);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpParams->pTarget[0]);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

    VPHAL_GET_SURFACE_INFO  info;

    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(
        m_pvpParams->pSrc[0],
        info));

    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(
        m_pvpParams->pTarget[0],
        info));

    if (!RECT1_CONTAINS_RECT2(m_pvpParams->pSrc[0]->rcMaxSrc, m_pvpParams->pSrc[0]->rcSrc))
    {
       m_pvpParams->pSrc[0]->rcMaxSrc = m_pvpParams->pSrc[0]->rcSrc;
    }

    bool bApgFuncSupported = false;
    VP_PUBLIC_CHK_STATUS_RETURN(CheckFeatures(params, bApgFuncSupported));

    if (!bApgFuncSupported)
    {
        VP_PUBLIC_NORMALMESSAGE("Features are not supported on APG now \n");

        if (m_currentFrameAPGEnabled)
        {
            m_pvpParams->bAPGWorkloadEnable = true;
            m_currentFrameAPGEnabled        = false;
        }

        return MOS_STATUS_UNIMPLEMENTED;
    }
    else
    {
        m_currentFrameAPGEnabled        = true;
        m_pvpParams->bAPGWorkloadEnable = false;
        VP_PUBLIC_NORMALMESSAGE("Features can be enabled on APG");
    }

    // Init Resource Max Rect for primary video

    if ((nullptr != m_pvpMhwInterface) &&
        (nullptr != m_pvpMhwInterface->m_osInterface) &&
        (nullptr != m_pvpMhwInterface->m_osInterface->osCpInterface))
    {
        for (uint32_t uiIndex = 0; uiIndex < m_pvpParams->uSrcCount; uiIndex++)
        {
            ppSource[uiIndex] = &(m_pvpParams->pSrc[uiIndex]->OsResource);
        }
        for (uint32_t uiIndex = 0; uiIndex < m_pvpParams->uDstCount; uiIndex++)
        {
            ppTarget[uiIndex] = &(m_pvpParams->pTarget[uiIndex]->OsResource);
        }
        m_pvpMhwInterface->m_osInterface->osCpInterface->PrepareResources(
            (void **)ppSource, m_pvpParams->uSrcCount, (void **)ppTarget, m_pvpParams->uDstCount);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::PrepareVpExePipe()
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpParams);

    // Get Output Pipe for Features
    // Output Pipe can be multiple submittion for example: Render + VE
    // Check the feasible for each engine, current noe for CSC/Scaling, we set it as SFC
    m_vpOutputPipe = VPHAL_OUTPUT_PIPE_MODE_SFC;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::SetVpPipelineMhwInterfce(void *mhwInterface)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(mhwInterface);

    m_pvpMhwInterface = (PVP_MHWINTERFACE)mhwInterface;

    return MOS_STATUS_SUCCESS;
}

} // namespace vp
