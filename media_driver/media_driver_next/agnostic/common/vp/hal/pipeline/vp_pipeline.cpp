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
#include "vp_platform_interface.h"
#include "vp_utils.h"
using namespace vp;

VpPipeline::VpPipeline(PMOS_INTERFACE osInterface) :
    MediaPipeline(osInterface)
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
    MOS_Delete(m_vpInterface);
    MOS_Delete(m_resourceManager);
    MOS_Delete(m_paramChecker);
    MOS_Delete(m_mmc);
    MOS_Delete(m_allocator);
    MOS_Delete(m_statusReport);
    MOS_Delete(m_packetSharedContext);
    MOS_Delete(m_reporting);

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

MOS_STATUS VpPipeline::Destroy()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::UserFeatureReport()
{
    VP_FUNC_CALL();

    if (m_reporting)
    {
        m_reporting->OutputPipeMode = m_vpOutputPipe;
        m_reporting->VEFeatureInUse = m_veboxFeatureInuse;

        if (m_mmc)
        {
            m_reporting->VPMMCInUse = m_mmc->IsMmcEnabled();
        }

        if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
        {
            PVP_PIPELINE_PARAMS params = m_pvpParams.renderParams;
            VP_PUBLIC_CHK_NULL_RETURN(params);
            if (params->pSrc[0] && params->pSrc[0]->bCompressible)
            {
                m_reporting->PrimaryCompressible = true;
                m_reporting->PrimaryCompressMode = (uint8_t)(params->pSrc[0]->CompressionMode);
            }

            if (params->pTarget[0]->bCompressible)
            {
                m_reporting->RTCompressible = true;
                m_reporting->RTCompressMode = (uint8_t)(params->pTarget[0]->CompressionMode);
            }
        }
    }

    MediaPipeline::UserFeatureReport();

#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_currentFrameAPGEnabled)
    {
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE_ID, 1, m_osInterface->pOsContext);
    }
    else
    {
        WriteUserFeature(__MEDIA_USER_FEATURE_VALUE_VPP_APOGEIOS_ENABLE_ID, 0, m_osInterface->pOsContext);
    }
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CreatePacketSharedContext()
{
    m_packetSharedContext = MOS_New(VP_PACKET_SHARED_CONTEXT);
    VP_PUBLIC_CHK_NULL_RETURN(m_packetSharedContext);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::Init(void *mhwInterface)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(mhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(((PVP_MHWINTERFACE)mhwInterface)->m_vpPlatformInterface);

    m_vpMhwInterface = *(PVP_MHWINTERFACE)mhwInterface;

    VP_PUBLIC_CHK_STATUS_RETURN(MediaPipeline::InitPlatform());

    VP_PUBLIC_CHK_STATUS_RETURN(CreateFeatureReport());

    m_mediaContext = MOS_New(MediaContext, scalabilityVp, &m_vpMhwInterface, m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_mediaContext);

    m_mmc = MOS_New(VPMediaMemComp, m_osInterface, &m_vpMhwInterface);
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

    m_pPacketFactory = MOS_New(PacketFactory, m_vpMhwInterface.m_vpPlatformInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketFactory);

    VP_PUBLIC_CHK_STATUS_RETURN(CreatePacketSharedContext());
    // Create active tasks
    MediaTask *pTask = GetTask(MediaTask::TaskType::cmdTask);
    VP_PUBLIC_CHK_NULL_RETURN(pTask);
    VP_PUBLIC_CHK_STATUS_RETURN(m_pPacketFactory->Initialize(pTask, &m_vpMhwInterface, m_allocator, m_mmc, m_packetSharedContext));

    m_pPacketPipeFactory = MOS_New(PacketPipeFactory, *m_pPacketFactory);
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketPipeFactory);

    VP_PUBLIC_CHK_STATUS_RETURN(GetSystemVeboxNumber());

    return MOS_STATUS_SUCCESS;
}

bool VpPipeline::IsVeboxSfcFormatSupported(MOS_FORMAT formatInput, MOS_FORMAT formatOutput)
{
    VpFeatureManagerNext *featureManagerNext = dynamic_cast<VpFeatureManagerNext *>(m_featureManager);
    if (nullptr == featureManagerNext)
    {
        VP_PUBLIC_ASSERTMESSAGE("m_featureManager equals to nullptr!");
        return false;
    }
    return featureManagerNext->IsVeboxSfcFormatSupported(formatInput, formatOutput);
}

MOS_STATUS VpPipeline::ExecuteVpPipeline()
{
    VP_FUNC_CALL();

    MOS_STATUS              eStatus   = MOS_STATUS_SUCCESS;
    PacketPipe              *pPacketPipe = nullptr;
    SwFilterPipe            *swFilterPipe = nullptr;
    VpFeatureManagerNext    *featureManagerNext = dynamic_cast<VpFeatureManagerNext *>(m_featureManager);

    VP_PUBLIC_CHK_NULL_RETURN(featureManagerNext);
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketPipeFactory);

    if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
    {
        PVP_PIPELINE_PARAMS params = m_pvpParams.renderParams;
        VP_PUBLIC_CHK_NULL(params);
        // Set Pipeline status Table
        m_statusReport->SetPipeStatusReportParams(params, m_vpMhwInterface.m_statusTable);

        VPHAL_PARAMETERS_DUMPPER_DUMP_XML(params);

        for (uint32_t uiLayer = 0; uiLayer < params->uSrcCount && uiLayer < VPHAL_MAX_SOURCES; uiLayer++)
        {
            VPHAL_SURFACE_DUMP(m_surfaceDumper,
                params->pSrc[uiLayer],
                m_frameCounter,
                uiLayer,
                VPHAL_DUMP_TYPE_PRE_ALL);
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(CreateSwFilterPipe(m_pvpParams, swFilterPipe));

    pPacketPipe = m_pPacketPipeFactory->CreatePacketPipe();
    VP_PUBLIC_CHK_NULL(pPacketPipe);

    eStatus = featureManagerNext->InitPacketPipe(*swFilterPipe, *pPacketPipe);
    m_vpInterface->GetSwFilterPipeFactory().Destory(swFilterPipe);
    VP_PUBLIC_CHK_STATUS(eStatus);

    // Update output pipe mode.
    m_vpOutputPipe = pPacketPipe->GetOutputPipeMode();
    m_veboxFeatureInuse = pPacketPipe->IsVeboxFeatureInuse();

    // MediaPipeline::m_statusReport is always nullptr in VP APO path right now.
    eStatus = pPacketPipe->Execute(MediaPipeline::m_statusReport, m_scalability, m_mediaContext, MOS_VE_SUPPORTED(m_osInterface), m_numVebox);

    m_pPacketPipeFactory->ReturnPacketPipe(pPacketPipe);

    if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
    {
        PVP_PIPELINE_PARAMS params = m_pvpParams.renderParams;
        VP_PUBLIC_CHK_NULL(params);
        VPHAL_SURFACE_PTRS_DUMP(m_surfaceDumper,
                            params->pTarget,
                            VPHAL_MAX_TARGETS,
                            params->uDstCount,
                            m_frameCounter,
                            VPHAL_DUMP_TYPE_POST_ALL);
    }

finish:
    m_pPacketPipeFactory->ReturnPacketPipe(pPacketPipe);
    m_vpInterface->GetSwFilterPipeFactory().Destory(swFilterPipe);
    m_statusReport->UpdateStatusTableAfterSubmit(eStatus);
    m_frameCounter++;
    return eStatus;
}

MOS_STATUS VpPipeline::CreateSwFilterPipe(VP_PARAMS &params, SwFilterPipe *&swFilterPipe)
{
    swFilterPipe = nullptr;

    switch (m_pvpParams.type)
    {
    case PIPELINE_PARAM_TYPE_LEGACY:
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface->GetSwFilterPipeFactory().Create(m_pvpParams.renderParams, swFilterPipe));
        break;
    case PIPELINE_PARAM_TYPE_MEDIA_SFC_INTERFACE:
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface->GetSwFilterPipeFactory().Create(m_pvpParams.sfcParams, swFilterPipe));
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        break;
    }

    VP_PUBLIC_CHK_NULL_RETURN(swFilterPipe);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::GetSystemVeboxNumber()
{
    // Check whether scalability being disabled.
    MOS_USER_FEATURE_VALUE_DATA userFeatureData;
    MOS_ZeroMemory(&userFeatureData, sizeof(userFeatureData));

    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;
    statusKey = MOS_UserFeature_ReadValue_ID(
        nullptr,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE_ID,
        &userFeatureData,
        m_osInterface->pOsContext);

    bool disableScalability = false;
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = userFeatureData.i32Data ? true : false;
    }

    if (disableScalability)
    {
        m_numVebox = 1;
        return MOS_STATUS_SUCCESS;
    }

    // Get vebox number from meida sys info.
    MEDIA_ENGINE_INFO mediaSysInfo = {};
    MOS_STATUS        eStatus      = m_osInterface->pfnGetMediaEngineInfo(m_osInterface, mediaSysInfo);
    if (MOS_SUCCEEDED(eStatus))
    {
        // Both VE mode and media solo mode should be able to get the VDBOX number via the same interface
        m_numVebox = (uint8_t)(mediaSysInfo.VEBoxInfo.NumberOfVEBoxEnabled);
        if (m_numVebox == 0)
        {
            VP_PUBLIC_ASSERTMESSAGE("Fail to get the m_numVebox with value 0");
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
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
    VP_PUBLIC_CHK_NULL_RETURN(m_reporting);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpMhwInterface.m_vpPlatformInterface);

    // Add CheckFeatures api later in FeatureManagerNext.
    m_paramChecker = m_vpMhwInterface.m_vpPlatformInterface->CreateFeatureChecker(&m_vpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_paramChecker);

    VP_PUBLIC_CHK_STATUS_RETURN(CreateResourceManager());

    m_vpInterface = MOS_New(VpInterface, &m_vpMhwInterface, *m_allocator, m_resourceManager);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface);

    m_featureManager = MOS_New(VpFeatureManagerNext, *m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

    VP_PUBLIC_CHK_STATUS_RETURN(((VpFeatureManagerNext *)m_featureManager)->Initialize());

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  create reource manager
//! \return MOS_STATUS
//!         MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpPipeline::CreateResourceManager()
{
    if (nullptr == m_resourceManager)
    {
        m_resourceManager = MOS_New(VpResourceManager, *m_osInterface, *m_allocator, *m_reporting);
        VP_PUBLIC_CHK_NULL_RETURN(m_resourceManager);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CheckFeatures(void *params, bool &bapgFuncSupported)
{
    VP_PUBLIC_CHK_NULL_RETURN(m_paramChecker);
    return m_paramChecker->CheckFeatures(params, bapgFuncSupported);
}

MOS_STATUS VpPipeline::CreateFeatureReport()
{
    if (m_reporting == nullptr)
    {
       m_reporting = MOS_New(VphalFeatureReport);
    }
    VP_PUBLIC_CHK_NULL_RETURN(m_reporting);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::PrepareVpPipelineParams(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(params);

    if ((m_vpMhwInterface.m_osInterface != nullptr))
    {
        // Set the component info
        m_vpMhwInterface.m_osInterface->Component = params->Component;

        // Init component(DDI entry point) info for perf measurement
        m_vpMhwInterface.m_osInterface->pfnSetPerfTag(m_vpMhwInterface.m_osInterface, VPHAL_NONE);
    }

    PMOS_RESOURCE ppSource[VPHAL_MAX_SOURCES] = {nullptr};
    PMOS_RESOURCE ppTarget[VPHAL_MAX_TARGETS] = {nullptr};

    if (!params->pSrc[0])
    {
        VP_PUBLIC_NORMALMESSAGE("Not support no source case in APG now \n");

        if (m_currentFrameAPGEnabled)
        {
            params->bAPGWorkloadEnable = true;
            m_currentFrameAPGEnabled = false;
        }
        else
        {
            params->bAPGWorkloadEnable = false;
        }

        return MOS_STATUS_UNIMPLEMENTED;
    }

    VP_PUBLIC_CHK_NULL_RETURN(params->pTarget[0]);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

    VPHAL_GET_SURFACE_INFO  info;

    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(
        params->pSrc[0],
        info));

    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(
        params->pTarget[0],
        info));

    if (!RECT1_CONTAINS_RECT2(params->pSrc[0]->rcMaxSrc, params->pSrc[0]->rcSrc))
    {
        params->pSrc[0]->rcMaxSrc = params->pSrc[0]->rcSrc;
    }

    bool bApgFuncSupported = false;
    VP_PUBLIC_CHK_STATUS_RETURN(CheckFeatures(params, bApgFuncSupported));
    if (!bApgFuncSupported)
    {
        VP_PUBLIC_NORMALMESSAGE("Features are not supported on APG now \n");

        if (m_currentFrameAPGEnabled)
        {
            params->bAPGWorkloadEnable = true;
            m_currentFrameAPGEnabled        = false;
        }
        else
        {
            params->bAPGWorkloadEnable = false;
        }

        return MOS_STATUS_UNIMPLEMENTED;
    }
    else
    {
        m_currentFrameAPGEnabled        = true;
        params->bAPGWorkloadEnable = false;
        VP_PUBLIC_NORMALMESSAGE("Features can be enabled on APG");
    }

    // Init Resource Max Rect for primary video

    if ((nullptr != m_vpMhwInterface.m_osInterface) &&
        (nullptr != m_vpMhwInterface.m_osInterface->osCpInterface))
    {
        for (uint32_t uiIndex = 0; uiIndex < params->uSrcCount; uiIndex++)
        {
            ppSource[uiIndex] = &(params->pSrc[uiIndex]->OsResource);
        }
        for (uint32_t uiIndex = 0; uiIndex < params->uDstCount; uiIndex++)
        {
            ppTarget[uiIndex] = &(params->pTarget[uiIndex]->OsResource);
        }
        m_vpMhwInterface.m_osInterface->osCpInterface->PrepareResources(
            (void **)ppSource, params->uSrcCount, (void **)ppTarget, params->uDstCount);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::Prepare(void * params)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);

    m_pvpParams = *(VP_PARAMS *)params;
    // Get Output Pipe for Features. It should be configured in ExecuteVpPipeline.
    m_vpOutputPipe = VPHAL_OUTPUT_PIPE_MODE_INVALID;
    m_veboxFeatureInuse = false;

    if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
    {
        // VP Execution Params Prepare
        eStatus = PrepareVpPipelineParams(m_pvpParams.renderParams);
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
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::Execute()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(ExecuteVpPipeline())
    VP_PUBLIC_CHK_STATUS_RETURN(UserFeatureReport());

    if (m_packetSharedContext && m_packetSharedContext->bFirstFrame)
    {
        m_packetSharedContext->bFirstFrame = false;
    }

    return MOS_STATUS_SUCCESS;
}
