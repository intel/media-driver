/*
* Copyright (c) 2018-2024, Intel Corporation
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
#include "vp_user_feature_control.h"
using namespace vp;

VpPipeline::VpPipeline(PMOS_INTERFACE osInterface) :
    MediaPipeline(osInterface)
{
}

VpPipeline::~VpPipeline()
{
#if (_DEBUG || _RELEASE_INTERNAL)
    if (m_reportOnceFlag)
    {
        ReportUserSettingForDebug(
            m_userSettingPtr,
            __MEDIA_USER_FEATURE_VALUE_FALLBACK_SCALING_TO_RENDER_8K_REPORT,
            0,
            MediaUserSetting::Group::Sequence);
    }
#endif
    // Delete m_featureManager before m_resourceManager, since
    // m_resourceManager is referenced by m_featureManager.
    MOS_Delete(m_featureManager);
    for (auto ctx : m_vpPipeContexts)
    {
        MOS_Delete(ctx);
    }
    m_vpPipeContexts.clear();
    // Delete m_pPacketPipeFactory before m_pPacketFactory, since
    // m_pPacketFactory is referenced by m_pPacketPipeFactory.
    MOS_Delete(m_pPacketPipeFactory);
    MOS_Delete(m_pPacketFactory);
    DeletePackets();
    DeleteTasks();

    MOS_Delete(m_vpInterface);
    MOS_Delete(m_kernelSet);
    MOS_Delete(m_paramChecker);
    MOS_Delete(m_mmc);
#if (_DEBUG || _RELEASE_INTERNAL)
    DestroySurface();
#endif
    MOS_Delete(m_allocator);
    MOS_Delete(m_statusReport);
    MOS_Delete(m_packetSharedContext);
    if (m_vpMhwInterface.m_reporting && this != m_vpMhwInterface.m_reporting->owner)
    {
        m_reporting = nullptr;
    }
    else
    {
        MOS_Delete(m_reporting);
        m_vpMhwInterface.m_reporting = nullptr;
    }
#if ((_DEBUG || _RELEASE_INTERNAL) && !EMUL)
    VP_DEBUG_INTERFACE_DESTROY(m_debugInterface);
#endif
    if (m_mediaContext)
    {
        MOS_Delete(m_mediaContext);
        m_mediaContext = nullptr;
    }

    if (m_vpSettings)
    {
        MOS_FreeMemory(m_vpSettings);
        m_vpSettings = nullptr;
    }

    if (m_userFeatureControl &&
        (this == m_userFeatureControl->m_owner || nullptr == m_userFeatureControl->m_owner))
    {
        MOS_Delete(m_userFeatureControl);
        m_vpMhwInterface.m_userFeatureControl = nullptr;
    }
    if (m_pipelineParamFactory)
    {
        MOS_Delete(m_pipelineParamFactory);
    }
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

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS VpPipeline::DestroySurface()
{
    VP_FUNC_CALL();

    if (m_tempTargetSurface)
    {
        m_allocator->FreeResource(&m_tempTargetSurface->OsResource);
        MOS_FreeMemAndSetNull(m_tempTargetSurface);
    }

    return MOS_STATUS_SUCCESS;
}
#endif

#if (_DEBUG || _RELEASE_INTERNAL)
MOS_STATUS VpPipeline::ReportIFNCC(bool bStart)
{
    //INTER_FRAME_MEMORY_NINJA_START_COUNTER will be reported in Prepare function
    //INTER_FRAME_MEMORY_NINJA_END_COUNTER will be reported in UserFeatureReport() function which runs in Execute()
    VP_FUNC_CALL();
    int32_t memninjaCounter = 0;
    memninjaCounter         = *MosUtilities::m_mosMemAllocCounter + *MosUtilities::m_mosMemAllocCounterGfx - *MosUtilities::m_mosMemAllocFakeCounter;
    if (MOS_IS_MEMORY_FOOT_PRINT_ENABLED())
    {
        if (bStart)
        {
            MT_LOG2(MT_MOS_MEMORY_NINJA_COUNTER, MT_NORMAL, MT_MEMORY_NINJA_IS_START, bStart, MT_MEMORY_NINJA_START_COUNTER, memninjaCounter);
            VP_PUBLIC_NORMALMESSAGE("MT_MOS_MEMORY_NINJA_COUNTER, MT_MEMORY_NINJA_IS_START :%d, MT_MEMORY_NINJA_START_COUNTER: %d", bStart, memninjaCounter);
        }
        else
        {
            MT_LOG2(MT_MOS_MEMORY_NINJA_COUNTER, MT_NORMAL, MT_MEMORY_NINJA_IS_START, bStart, MT_MEMORY_NINJA_END_COUNTER, memninjaCounter);
            VP_PUBLIC_NORMALMESSAGE("MT_MOS_MEMORY_NINJA_COUNTER, MT_MEMORY_NINJA_IS_START :%d, MT_MEMORY_NINJA_END_COUNTER: %d", bStart, memninjaCounter);
        }
    }
    if (m_userFeatureControl->EnableIFNCC() &&
        MosUtilities::m_mosMemAllocCounter &&
        MosUtilities::m_mosMemAllocCounterGfx &&
        MosUtilities::m_mosMemAllocFakeCounter)
    {
        ReportUserSettingForDebug(
            m_userSettingPtr,
            bStart ? __MEDIA_USER_FEATURE_VALUE_INTER_FRAME_MEMORY_NINJA_START_COUNTER : __MEDIA_USER_FEATURE_VALUE_INTER_FRAME_MEMORY_NINJA_END_COUNTER,
            memninjaCounter,
            MediaUserSetting::Group::Sequence);
    }
    return MOS_STATUS_SUCCESS;
}
#endif

MOS_STATUS VpPipeline::UserFeatureReport()
{
    VP_FUNC_CALL();

    if (m_reporting)
    {
        if (m_vpPipeContexts.size() > 0)
        {
            VP_PUBLIC_CHK_NULL_RETURN(m_vpPipeContexts[0]);
            m_reporting->GetFeatures().outputPipeMode = m_vpPipeContexts[0]->GetOutputPipe();
            m_reporting->GetFeatures().veFeatureInUse = m_vpPipeContexts[0]->IsVeboxInUse();
            m_reporting->GetFeatures().packetReused   = m_vpPipeContexts[0]->IsPacketReUsed();
        }

        if (m_mmc)
        {
            m_reporting->GetFeatures().vpMMCInUse = m_mmc->IsMmcEnabled();
        }

        if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
        {
            PVP_PIPELINE_PARAMS params = m_pvpParams.renderParams;
            VP_PUBLIC_CHK_NULL_RETURN(params);
            if (params->pSrc[0] && params->pSrc[0]->bCompressible)
            {
                m_reporting->GetFeatures().primaryCompressible = true;
                m_reporting->GetFeatures().primaryCompressMode = (uint8_t)(params->pSrc[0]->CompressionMode);
            }

            if (params->pTarget[0]->bCompressible)
            {
                m_reporting->GetFeatures().rtCompressible = true;
                m_reporting->GetFeatures().rtCompressMode = (uint8_t)(params->pTarget[0]->CompressionMode);
            }
            
            m_reporting->GetFeatures().rtCacheSetting = (uint8_t)(params->pTarget[0]->CacheSetting);
#if (_DEBUG || _RELEASE_INTERNAL)
            if (m_reporting->GetFeatures().outputPipeMode == VPHAL_OUTPUT_PIPE_MODE_SFC)
            {
                if (m_vpMhwInterface.m_sfcInterface)
                {
                    m_reporting->GetFeatures().rtOldCacheSetting = (uint8_t)((m_vpMhwInterface.m_sfcInterface->m_outputSurfCtrl.Value >> 1) & 0x0000003f);
                }
            }
            else if (m_vpMhwInterface.m_renderHal)
            {
                m_reporting->GetFeatures().rtOldCacheSetting = (uint8_t)(m_vpMhwInterface.m_renderHal->oldCacheSettingForTargetSurface);
            }
            if (m_reporting->GetFeatures().isOcl3DLut)
            {
                VP_PUBLIC_NORMALMESSAGE("VP OCL 3DLut Enabled");
                ReportUserSettingForDebug(
                    m_userSettingPtr,
                    __MEDIA_USER_FEATURE_VALUE_VP_OCL_3DLUT_ENABLED,
                    1,
                    MediaUserSetting::Group::Sequence);
            }
#endif
        }
#if (_DEBUG || _RELEASE_INTERNAL)
        if (m_reporting->GetFeatures().isOclFC)
        {
            VP_PUBLIC_NORMALMESSAGE("VP OCL FC Supported");
            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_SUPPORTED,
                1,
                MediaUserSetting::Group::Sequence);

            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_FEATURE_REPORT,
                m_reporting->GetFeatures().featureLogOclFC,
                MediaUserSetting::Group::Sequence);

            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_VP_OCL_FC_REPORT,
                m_reporting->GetFeatures().diffLogOclFC,
                MediaUserSetting::Group::Sequence);

            if (m_reporting->GetFeatures().isLegacyFCInUse)
            {
                ReportUserSettingForDebug(
                    m_userSettingPtr,
                    __MEDIA_USER_FEATURE_VALUE_VP_LEGACY_FC_IN_USE,
                    1,
                    MediaUserSetting::Group::Sequence);
                m_reporting->GetFeatures().isLegacyFCInUse = false;
            }
        }

        if (m_reportOnceFlag && m_reporting->GetFeatures().fallbackScalingToRender8K)
        {
            ReportUserSettingForDebug(
                m_userSettingPtr,
                __MEDIA_USER_FEATURE_VALUE_FALLBACK_SCALING_TO_RENDER_8K_REPORT,
                1,
                MediaUserSetting::Group::Sequence);
            m_reporting->GetFeatures().fallbackScalingToRender8K = false;
            m_reportOnceFlag                                     = false;
        }
        
#endif

        m_reporting->GetFeatures().VPApogeios = m_currentFrameAPGEnabled;
    }
    MediaPipeline::UserFeatureReport();


#if (_DEBUG || _RELEASE_INTERNAL)
    //INTER_FRAME_MEMORY_NINJA_START_COUNTER will be reported in ReportIFNCC(true) function which runs in VpPipeline::Prepare()
    ReportIFNCC(false);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CreatePacketSharedContext()
{
    VP_FUNC_CALL();

    m_packetSharedContext = MOS_New(VP_PACKET_SHARED_CONTEXT);
    VP_PUBLIC_CHK_NULL_RETURN(m_packetSharedContext);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CreateUserFeatureControl()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    m_userFeatureControl = MOS_New(VpUserFeatureControl, *m_osInterface, m_vpMhwInterface.m_vpPlatformInterface, this);
    VP_PUBLIC_CHK_NULL_RETURN(m_userFeatureControl);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CreateVPDebugInterface()
{
    VP_FUNC_CALL();

#if ((_DEBUG || _RELEASE_INTERNAL) && !EMUL)
    VP_DEBUG_INTERFACE_CREATE(m_debugInterface);
    SkuWaTable_DUMP_XML(m_skuTable, m_waTable);
#endif
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::Init(void *mhwInterface)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(mhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(((PVP_MHWINTERFACE)mhwInterface)->m_vpPlatformInterface);

    m_vpMhwInterface = *(PVP_MHWINTERFACE)mhwInterface;

    if (m_vpMhwInterface.m_userFeatureControl)
    {
        m_userFeatureControl = m_vpMhwInterface.m_userFeatureControl;
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(CreateUserFeatureControl());
        m_vpMhwInterface.m_userFeatureControl = m_userFeatureControl;
    }
    if (m_userFeatureControl && m_vpMhwInterface.m_settings)
    {
        m_userFeatureControl->SetClearVideoViewMode(((VP_SETTINGS *)m_vpMhwInterface.m_settings)->clearVideoViewMode);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(m_vpMhwInterface.m_vpPlatformInterface->ConfigVirtualEngine());

    VP_PUBLIC_CHK_STATUS_RETURN(m_vpMhwInterface.m_vpPlatformInterface->ConfigureVpScalability(m_vpMhwInterface));

    VP_PUBLIC_CHK_STATUS_RETURN(MediaPipeline::InitPlatform());

    VP_PUBLIC_CHK_STATUS_RETURN(MediaPipeline::CreateMediaCopyWrapper());

    VP_PUBLIC_CHK_NULL_RETURN(m_mediaCopyWrapper);

    if (m_mediaCopyWrapper->MediaCopyStateIsNull())
    {
        // Only for legacy media copy creation
        m_mediaCopyWrapper->SetMediaCopyState(m_vpMhwInterface.m_vpPlatformInterface->CreateMediaCopy());
    }

    VP_PUBLIC_CHK_STATUS_RETURN(CreateFeatureReport());

    m_mediaContext = MOS_New(MediaContext, scalabilityVp, &m_vpMhwInterface, m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_mediaContext);

    m_mmc = MOS_New(VPMediaMemComp, m_osInterface, m_vpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_mmc);

    m_allocator = MOS_New(VpAllocator, m_osInterface, m_mmc);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);

    m_statusReport = MOS_New(VPStatusReport, m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_statusReport);

    VP_PUBLIC_CHK_STATUS_RETURN(CreateVPDebugInterface());

    m_vpMhwInterface.m_debugInterface = (void*)m_debugInterface;

    m_pPacketFactory = MOS_New(PacketFactory, m_vpMhwInterface.m_vpPlatformInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketFactory);

    VP_PUBLIC_CHK_STATUS_RETURN(CreatePacketSharedContext());
    VP_PUBLIC_CHK_STATUS_RETURN(CreateVpKernelSets());
    // Create active tasks
    MediaTask *pTask = GetTask(MediaTask::TaskType::cmdTask);
    VP_PUBLIC_CHK_NULL_RETURN(pTask);
    VP_PUBLIC_CHK_STATUS_RETURN(m_pPacketFactory->Initialize(pTask, &m_vpMhwInterface, m_allocator, m_mmc, m_packetSharedContext, m_kernelSet, m_debugInterface));

    m_pPacketPipeFactory = MOS_New(PacketPipeFactory, *m_pPacketFactory);
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketPipeFactory);

    if (m_vpPipeContexts.size() == 0)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(CreateSinglePipeContext());
    }
    VpResourceManager *resourceManager = m_vpPipeContexts[0]->GetVpResourceManager();
    VP_PUBLIC_CHK_STATUS_RETURN(CreateFeatureManager(resourceManager));
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

    VP_PUBLIC_CHK_STATUS_RETURN(GetSystemVeboxNumber());

    VP_PUBLIC_CHK_STATUS_RETURN(SetVideoProcessingSettings(m_vpMhwInterface.m_settings));

    m_vpMhwInterface.m_settings = m_vpSettings;
    if (m_vpMhwInterface.m_vpPlatformInterface->IsGpuContextCreatedInPipelineInit())
    {
        if (m_numVebox > 0 && !(m_vpSettings && m_vpSettings->clearVideoViewMode))
        {
            VP_PUBLIC_NORMALMESSAGE("Create GpuContext for Vebox.");
            VP_PUBLIC_CHK_STATUS_RETURN(PacketPipe::SwitchContext(VP_PIPELINE_PACKET_VEBOX, m_scalability,
                m_mediaContext, MOS_VE_SUPPORTED(m_osInterface), m_numVebox));
        }
        // If the environment is SA media in which there are no GT IP, we could not create/use Render or Compute GPU context.
        if (!(m_skuTable && MEDIA_IS_SKU(m_skuTable, FtrDisableGtIpSubmissions)))
        {
            bool computeContextEnabled = m_userFeatureControl->IsComputeContextEnabled();
            auto packetId              = computeContextEnabled ? VP_PIPELINE_PACKET_COMPUTE : VP_PIPELINE_PACKET_RENDER;
            VP_PUBLIC_NORMALMESSAGE("Create GpuContext for Compute/Render (PacketId: %d).", packetId);
            VP_PUBLIC_CHK_STATUS_RETURN(PacketPipe::SwitchContext(packetId, m_scalability,
                m_mediaContext, MOS_VE_SUPPORTED(m_osInterface), m_numVebox));
        }
        // create SinglePipe GpuContext for multi Vebox system to avoid first frame long latency issue
        if (m_numVebox > 1 && !(m_vpSettings && m_vpSettings->clearVideoViewMode))
        {
            VP_PUBLIC_NORMALMESSAGE("Create Single Pipe GpuContext for Vebox.");
            VP_PUBLIC_CHK_STATUS_RETURN(PacketPipe::SwitchContext(VP_PIPELINE_PACKET_VEBOX, m_scalability,
                m_mediaContext, MOS_VE_SUPPORTED(m_osInterface), 1));
        }
    }

    return MOS_STATUS_SUCCESS;
}

bool VpPipeline::IsVeboxSfcFormatSupported(MOS_FORMAT formatInput, MOS_FORMAT formatOutput)
{
    VP_FUNC_CALL();

    VpFeatureManagerNext *featureManagerNext = dynamic_cast<VpFeatureManagerNext *>(m_featureManager);
    if (nullptr == featureManagerNext)
    {
        VP_PUBLIC_ASSERTMESSAGE("m_featureManager equals to nullptr!");
        return false;
    }
    return featureManagerNext->IsVeboxSfcFormatSupported(formatInput, formatOutput);
}

MOS_STATUS VpPipeline::UpdateRectForNegtiveDstTopLeft(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();

    for (uint32_t index = 0; (index < params->uSrcCount) && (index < VPHAL_MAX_SOURCES); index++)
    {
        PVPHAL_SURFACE pcSrc = params->pSrc[index];

        if (pcSrc)
        {
            if (pcSrc->rcDst.top < 0 || pcSrc->rcDst.left < 0)
            {
                VP_PUBLIC_NORMALMESSAGE("negtive value on rcDst top or left, top: %d, left: %d.", pcSrc->rcDst.top, pcSrc->rcDst.left);
                bool isVerticalRotation = VpUtils::IsVerticalRotation(pcSrc->Rotation);

                uint32_t srcHeight = pcSrc->rcSrc.bottom - pcSrc->rcSrc.top;
                uint32_t srcWidth  = pcSrc->rcSrc.right - pcSrc->rcSrc.left;
                uint32_t dstHeight = pcSrc->rcDst.bottom - pcSrc->rcDst.top;
                uint32_t dstWidth  = pcSrc->rcDst.right - pcSrc->rcDst.left;

                float fScaleX = isVerticalRotation ? (float)dstHeight / (float)srcWidth : (float)dstWidth / (float)srcWidth;
                float fScaleY = isVerticalRotation ? (float)dstWidth / (float)srcHeight : (float)dstHeight / (float)srcHeight;

                if (pcSrc->rcDst.top < 0)
                {
                    pcSrc->rcDst.top = 0;

                    if (isVerticalRotation)
                    {
                        uint32_t newDstHight = pcSrc->rcDst.bottom;
                        uint32_t newSrcWidth = MOS_UF_ROUND(newDstHight / fScaleX);
                        pcSrc->rcSrc.left    = pcSrc->rcSrc.right - newSrcWidth;
                    }
                    else
                    {
                        uint32_t newDstHight = pcSrc->rcDst.bottom;
                        uint32_t newSrcHight = MOS_UF_ROUND(newDstHight / fScaleY);
                        pcSrc->rcSrc.top     = pcSrc->rcSrc.bottom - newSrcHight;
                    }
                }
                if (pcSrc->rcDst.left < 0)
                {
                    pcSrc->rcDst.left = 0;
                    if (isVerticalRotation)
                    {
                        uint32_t newDstWidth = pcSrc->rcDst.right;
                        uint32_t newSrcHight = MOS_UF_ROUND(newDstWidth / fScaleY);
                        pcSrc->rcSrc.top     = pcSrc->rcSrc.bottom - newSrcHight;
                    }
                    else
                    {
                        uint32_t newDstWidth = pcSrc->rcDst.right;
                        uint32_t newSrcWidth = MOS_UF_ROUND(newDstWidth / fScaleX);
                        pcSrc->rcSrc.left    = pcSrc->rcSrc.right - newSrcWidth;
                    }
                }
                VP_PUBLIC_NORMALMESSAGE("updated source rectangle region: [%d,%d,%d,%d].", pcSrc->rcSrc.left, pcSrc->rcSrc.top, pcSrc->rcSrc.right, pcSrc->rcSrc.bottom);
                VP_PUBLIC_NORMALMESSAGE("updated destination rectangle region: [%d,%d,%d,%d].", pcSrc->rcDst.left, pcSrc->rcDst.top, pcSrc->rcDst.right, pcSrc->rcDst.bottom);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::ExecuteVpPipeline()
{
    VP_FUNC_CALL();

    MOS_STATUS                 eStatus   = MOS_STATUS_SUCCESS;
    PacketPipe                 *pPacketPipe = nullptr;
    std::vector<SwFilterPipe*> swFilterPipes;
    VpFeatureManagerNext       *featureManagerNext = dynamic_cast<VpFeatureManagerNext *>(m_featureManager);
    bool                       isBypassNeeded     = false;
    PVP_PIPELINE_PARAMS        params = nullptr;
    Policy                     *policy = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(featureManagerNext);
    VP_PUBLIC_CHK_NULL_RETURN(m_pPacketPipeFactory);
    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);

    if (m_vpPipeContexts.size() < 1 || m_vpPipeContexts[0] == nullptr)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    MT_LOG1(MT_VP_FEATURE_GRAPH_EXECUTE_VPPIPELINE_START, MT_NORMAL,
            MT_VP_FEATURE_GRAPH_FILTER_SWFILTERPIPE_COUNT, (int64_t)swFilterPipes.size());
    VP_PUBLIC_NORMALMESSAGE("Feature Graph: Execute VP Pipeline Start, swfilterPipes count:%d", (int64_t)swFilterPipes.size());

    if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
    {
        params = m_pvpParams.renderParams;
        VP_PUBLIC_CHK_NULL_RETURN(params);
        // Get the OptimizeCpuTiming flag from params
        m_osInterface->bOptimizeCpuTiming = params->bOptimizeCpuTiming;
        // Set Pipeline status Table
        m_statusReport->SetPipeStatusReportParams(params, m_vpMhwInterface.m_statusTable);

#if ((_DEBUG || _RELEASE_INTERNAL) && !EMUL)
        VP_PARAMETERS_DUMPPER_DUMP_XML(m_debugInterface,
            params,
            m_vpPipeContexts[0]->GetFrameCounter());

        for (uint32_t uiLayer = 0; uiLayer < params->uSrcCount && uiLayer < VPHAL_MAX_SOURCES; uiLayer++)
        {
            VP_SURFACE_DUMP(m_debugInterface,
                params->pSrc[uiLayer],
                m_vpPipeContexts[0]->GetFrameCounter(),
                uiLayer,
                VPHAL_DUMP_TYPE_PRE_ALL,
                VPHAL_SURF_DUMP_DDI_VP_BLT);
        }
#endif
        UpdateRectForNegtiveDstTopLeft(params);
        // Predication
        SetPredicationParams(params);

        eStatus = ProcessBypassHandler(params, isBypassNeeded);
        VP_PUBLIC_CHK_STATUS_RETURN(eStatus);
        if (isBypassNeeded)
        {
            MT_LOG2(MT_VP_FEATURE_GRAPH_EXECUTE_VPPIPELINE_END, MT_NORMAL, 
                    MT_VP_FEATURE_GRAPH_FILTER_SWFILTERPIPE_COUNT, (int64_t)swFilterPipes.size(),
                    MT_VP_FEATURE_GRAPH_FILTER_PIPELINEBYPASS, isBypassNeeded);
            VP_PUBLIC_NORMALMESSAGE("Feature Graph: Execute VP Pipeline End, swfilterPipes count:%d, isBypassNeeded:%d",
                                     (int64_t)swFilterPipes.size(),
                                     isBypassNeeded);
            return MOS_STATUS_SUCCESS;
        }
    }
    VP_PUBLIC_CHK_STATUS_RETURN(UpdateFrameTracker());
    VP_PUBLIC_CHK_STATUS_RETURN(CreateSwFilterPipe(m_pvpParams, swFilterPipes));

    // Increment frame ID for performance measurement
    m_osInterface->pfnIncPerfFrameID(m_osInterface);

    for (uint32_t pipeIdx = 0; pipeIdx < swFilterPipes.size(); pipeIdx++)
    {
        auto &pipe = swFilterPipes[pipeIdx];
        if (pipeIdx >= m_vpPipeContexts.size())
        {
            VP_PUBLIC_CHK_STATUS_RETURN(CreateSinglePipeContext());
        }
        MT_LOG2(MT_VP_FEATURE_GRAPH_EXECUTE_SINGLE_VPPIPELINE_START, MT_NORMAL,
                MT_VP_FEATURE_GRAPH_FILTER_FRAMEID, m_vpPipeContexts[pipeIdx]->GetFrameCounter(),
                MT_VP_FEATURE_GRAPH_FILTER_PIPEID, pipeIdx);
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: Execute VP Single Pipeline Start, frameid:%d, pipeid:%d",
                                 m_vpPipeContexts[pipeIdx]->GetFrameCounter(),
                                 pipeIdx);
        if (pipe)
        {
            pipe->AddRTLog();
        }
        auto &singlePipeCtx = m_vpPipeContexts[pipeIdx];
        VP_PUBLIC_CHK_NULL_RETURN(singlePipeCtx->GetVpResourceManager());
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface->SwitchResourceManager(singlePipeCtx->GetVpResourceManager()));

        VP_PUBLIC_CHK_STATUS_RETURN(ExecuteSingleswFilterPipe(singlePipeCtx, pipe, pPacketPipe, featureManagerNext));
        // FrameCounter will be increased inside ExecuteSingleswFilterPipe, so m_vpPipeContexts[pipeIdx]->GetFrameCounter() - 1 is needed.
        MT_LOG2(MT_VP_FEATURE_GRAPH_EXECUTE_SINGLE_VPPIPELINE_END, MT_NORMAL,
                MT_VP_FEATURE_GRAPH_FILTER_FRAMEID, m_vpPipeContexts[pipeIdx]->GetFrameCounter() - 1,
                MT_VP_FEATURE_GRAPH_FILTER_PIPEID, pipeIdx);
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: Execute VP Single Pipeline End, frameid:%d, pipeid:%d",
                                 m_vpPipeContexts[pipeIdx]->GetFrameCounter() - 1,
                                 pipeIdx);
    }

    MT_LOG2(MT_VP_FEATURE_GRAPH_EXECUTE_VPPIPELINE_END, MT_NORMAL,
            MT_VP_FEATURE_GRAPH_FILTER_SWFILTERPIPE_COUNT, (int64_t)swFilterPipes.size(),
            MT_VP_FEATURE_GRAPH_FILTER_PIPELINEBYPASS, isBypassNeeded);
    VP_PUBLIC_NORMALMESSAGE("Feature Graph: Execute VP Pipeline End, swfilterPipes count:%d, isBypassNeeded:%d",
                             (int64_t)swFilterPipes.size(),
                             isBypassNeeded);
    return eStatus;
}

MOS_STATUS VpPipeline::ExecuteSingleswFilterPipe(VpSinglePipeContext *singlePipeCtx, SwFilterPipe *&pipe, PacketPipe *pPacketPipe, VpFeatureManagerNext *featureManagerNext)
{
    VP_FUNC_CALL();
    VpResourceManager    *resourceManager = singlePipeCtx->GetVpResourceManager();
    VpPacketReuseManager *packetReuseMgr  = singlePipeCtx->GetPacketReUseManager();
    uint32_t              frameCounter    = singlePipeCtx->GetFrameCounter();
    MOS_STATUS            eStatus         = MOS_STATUS_SUCCESS;

    auto retHandler = [&]() {
        m_pPacketPipeFactory->ReturnPacketPipe(pPacketPipe);
        m_vpInterface->GetSwFilterPipeFactory().Destory(pipe);
        m_statusReport->UpdateStatusTableAfterSubmit(eStatus);
        // Notify resourceManager for end of new frame processing.
        resourceManager->OnNewFrameProcessEnd();
        MT_LOG1(MT_VP_HAL_ONNEWFRAME_PROC_END, MT_NORMAL, MT_VP_HAL_ONNEWFRAME_COUNTER, frameCounter);
        singlePipeCtx->AddFrameCount();
    };

    auto chkStatusHandler = [&](MOS_STATUS status) {
        if (MOS_FAILED(status))
        {
            retHandler();
        }
        return status;
    };

    auto chkNullHandler = [&](void *p) {
        if (nullptr == p)
        {
            retHandler();
        }
        return p;
    };

    // Notify resourceManager for start of new frame processing.
    MT_LOG1(MT_VP_HAL_ONNEWFRAME_PROC_START, MT_NORMAL, MT_VP_HAL_ONNEWFRAME_COUNTER, frameCounter);
    VP_PUBLIC_CHK_STATUS_RETURN(chkStatusHandler(resourceManager->OnNewFrameProcessStart(*pipe)));

    Policy *policy = featureManagerNext->GetPolicy();
    VP_PUBLIC_CHK_NULL_RETURN(chkNullHandler(policy));

    bool isPacketPipeReused = false;
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpParams.renderParams);
    VP_PUBLIC_CHK_STATUS_RETURN(chkStatusHandler(packetReuseMgr->PreparePacketPipeReuse(pipe, *policy, *resourceManager, isPacketPipeReused, m_pvpParams.renderParams->bOptimizeCpuTiming)));

    if (isPacketPipeReused)
    {
        VP_PUBLIC_NORMALMESSAGE("Packet reused.");
        MT_LOG(MT_VP_FEATURE_GRAPH_FEATUREPIPE_REUSE, MT_NORMAL);
        singlePipeCtx->SetPacketReused(true);

        PacketPipe *pipeReused = packetReuseMgr->GetPacketPipeReused();
        VP_PUBLIC_CHK_NULL_RETURN(chkNullHandler(pipeReused));

        // Update output pipe mode.
        singlePipeCtx->SetOutputPipeMode(pipeReused->GetOutputPipeMode());
        singlePipeCtx->SetIsVeboxFeatureInuse(pipeReused->IsVeboxFeatureInuse());
        // MediaPipeline::m_statusReport is always nullptr in VP APO path right now.
        eStatus = pipeReused->Execute(MediaPipeline::m_statusReport, m_scalability, m_mediaContext, MOS_VE_SUPPORTED(m_osInterface), m_numVebox);
        MT_LOG1(MT_VP_HAL_VEBOXNUM_CHECK, MT_NORMAL, MT_VP_HAL_VEBOX_NUMBER, m_numVebox)
        VP_PUBLIC_NORMALMESSAGE("Vebox Number for check %d", m_numVebox);
        if (MOS_SUCCEEDED(eStatus))
        {
            VP_PUBLIC_CHK_STATUS_RETURN(chkStatusHandler(UpdateExecuteStatus(frameCounter)));
        }

        m_vpInterface->GetSwFilterPipeFactory().Destory(pipe);

        m_statusReport->UpdateStatusTableAfterSubmit(eStatus);
        // Notify resourceManager for end of new frame processing.
        resourceManager->OnNewFrameProcessEnd();
        MT_LOG1(MT_VP_HAL_ONNEWFRAME_PROC_END, MT_NORMAL, MT_VP_HAL_ONNEWFRAME_COUNTER, frameCounter);
        singlePipeCtx->AddFrameCount();
        return eStatus;
    }
    else
    {
        singlePipeCtx->SetPacketReused(false);
    }

    pPacketPipe = m_pPacketPipeFactory->CreatePacketPipe();
    VP_PUBLIC_CHK_NULL_RETURN(chkNullHandler(pPacketPipe));

    eStatus = featureManagerNext->InitPacketPipe(*pipe, *pPacketPipe);
    m_vpInterface->GetSwFilterPipeFactory().Destory(pipe);
    VP_PUBLIC_CHK_STATUS_RETURN(chkStatusHandler(eStatus));

    // Update output pipe mode.
    singlePipeCtx->SetOutputPipeMode(pPacketPipe->GetOutputPipeMode());
    singlePipeCtx->SetIsVeboxFeatureInuse(pPacketPipe->IsVeboxFeatureInuse());

    // MediaPipeline::m_statusReport is always nullptr in VP APO path right now.
    eStatus = pPacketPipe->Execute(MediaPipeline::m_statusReport, m_scalability, m_mediaContext, MOS_VE_SUPPORTED(m_osInterface), m_numVebox);
    MT_LOG1(MT_VP_HAL_VEBOXNUM_CHECK, MT_NORMAL, MT_VP_HAL_VEBOX_NUMBER, m_numVebox)
    VP_PUBLIC_NORMALMESSAGE("Vebox Number for check %d", m_numVebox);
    if (MOS_SUCCEEDED(eStatus))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(chkStatusHandler(packetReuseMgr->UpdatePacketPipeConfig(pPacketPipe)));
        VP_PUBLIC_CHK_STATUS_RETURN(chkStatusHandler(UpdateExecuteStatus(frameCounter)));
    }

    m_pPacketPipeFactory->ReturnPacketPipe(pPacketPipe);
    retHandler();

    return eStatus;
}

MOS_STATUS VpPipeline::UpdateExecuteStatus(uint32_t frameCnt)
{
    VP_FUNC_CALL();

    MOS_STATUS eStatus = MOS_STATUS_SUCCESS;
#if ((_DEBUG || _RELEASE_INTERNAL))
    if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
    {
        PVP_PIPELINE_PARAMS params = m_pvpParams.renderParams;
        VP_PUBLIC_CHK_NULL(params);
        VP_SURFACE_PTRS_DUMP(m_debugInterface,
            params->pTarget,
            VPHAL_MAX_TARGETS,
            params->uDstCount,
            frameCnt,
            VPHAL_DUMP_TYPE_POST_ALL,
            params->uSrcCount > 0 ? VPHAL_SURF_DUMP_DDI_VP_BLT : VPHAL_SURF_DUMP_DDI_CLEAR_VIEW);

        // Decompre output surface for debug
        bool uiForceDecompressedOutput = false;
        uiForceDecompressedOutput = m_userFeatureControl->IsForceDecompressedOutput();

        if (uiForceDecompressedOutput)
        {
            VP_PUBLIC_NORMALMESSAGE("uiForceDecompressedOutput: %d", uiForceDecompressedOutput);
            m_mmc->DecompressVPResource(params->pTarget[0]);
        }
    }
finish:
#endif
    return eStatus;
}

MOS_STATUS VpPipeline::CreateSwFilterPipe(VP_PARAMS &params, std::vector<SwFilterPipe*> &swFilterPipe)
{
    VP_FUNC_CALL();
    switch (m_pvpParams.type)
    {
    case PIPELINE_PARAM_TYPE_LEGACY:
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface->GetSwFilterPipeFactory().Create(m_pvpParams.renderParams, swFilterPipe, m_pipelineParamFactory));
        break;
    case PIPELINE_PARAM_TYPE_MEDIA_SFC_INTERFACE:
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface->GetSwFilterPipeFactory().Create(m_pvpParams.sfcParams, swFilterPipe));
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    if (swFilterPipe.size() == 0)
    {
        VP_PUBLIC_ASSERTMESSAGE("Fail to create SwFilterPipe.");
        return MOS_STATUS_NULL_POINTER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::UpdateFrameTracker()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_vpMhwInterface.m_vpPlatformInterface);
    VP_PUBLIC_CHK_STATUS_RETURN(m_vpMhwInterface.m_vpPlatformInterface->InitFrameTracker());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::UpdateVeboxNumberforScalability()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_vpMhwInterface.m_vpPlatformInterface);

    // Check whether scalability being disabled.
    int32_t enableVeboxScalability = 0;

    if (m_numVebox <= 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Vebox Number of Enabled %d", m_numVebox);
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS statusKey = MOS_STATUS_SUCCESS;
    statusKey = ReadUserSetting(
        m_userSettingPtr,
        enableVeboxScalability,
        __MEDIA_USER_FEATURE_VALUE_ENABLE_VEBOX_SCALABILITY_MODE,
        MediaUserSetting::Group::Device);

    bool disableScalability = false;
    if (statusKey == MOS_STATUS_SUCCESS)
    {
        disableScalability = enableVeboxScalability ? false : true;
        if (disableScalability == false)
        {
            m_forceMultiplePipe = MOS_SCALABILITY_ENABLE_MODE_USER_FORCE | MOS_SCALABILITY_ENABLE_MODE_DEFAULT;
        }
        else
        {
            m_forceMultiplePipe = MOS_SCALABILITY_ENABLE_MODE_USER_FORCE | MOS_SCALABILITY_ENABLE_MODE_FALSE;
        }
    }
    else
    {
        m_forceMultiplePipe = MOS_SCALABILITY_ENABLE_MODE_DEFAULT;
    }

    if (disableScalability == true)
    {
        m_numVebox = 1;
        VP_PUBLIC_NORMALMESSAGE("DisableScalability Vebox Number of Enabled %d", m_numVebox);
        return MOS_STATUS_SUCCESS;
    }
    else if (m_forceMultiplePipe == MOS_SCALABILITY_ENABLE_MODE_DEFAULT)
    {
        if (m_vpMhwInterface.m_vpPlatformInterface->IsVeboxScalabilityWith4KNotSupported(m_vpMhwInterface) == true)
        {
            m_numVebox = 1;
            VP_PUBLIC_NORMALMESSAGE("ForceMultiplePipe Vebox Number of Enabled %d", m_numVebox);
            return MOS_STATUS_SUCCESS;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::GetSystemVeboxNumber()
{
    VP_FUNC_CALL();

    // Get vebox number from meida sys info.
    MEDIA_ENGINE_INFO mediaSysInfo = {};
    MOS_STATUS        eStatus      = m_osInterface->pfnGetMediaEngineInfo(m_osInterface, mediaSysInfo);
    if (MOS_SUCCEEDED(eStatus))
    {
        // Both VE mode and media solo mode should be able to get the VEBOX number via the same interface
        m_numVebox = (uint8_t)(mediaSysInfo.VEBoxInfo.NumberOfVEBoxEnabled);
        VP_PUBLIC_NORMALMESSAGE("Vebox Number of Enabled %d", m_numVebox);
        if (m_numVebox == 0 && !IsGtEnv())
        {
            VP_PUBLIC_ASSERTMESSAGE("Fail to get the m_numVebox with value 0");
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        else if (m_numVebox == 0 && MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrVERing))
        {
            m_numVebox = 1;
        }
    }
    else
    {
        m_numVebox = 1;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateVeboxNumberforScalability());

    m_numVeboxOriginal = m_numVebox;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CreateSinglePipeContext()
{
    VP_FUNC_CALL();
    VpSinglePipeContext *singlePipeCtx = MOS_New(VpSinglePipeContext);
    VP_PUBLIC_CHK_NULL_RETURN(singlePipeCtx);
    MOS_STATUS status = singlePipeCtx->Init(m_osInterface, m_allocator, m_reporting, m_vpMhwInterface.m_vpPlatformInterface, m_pPacketPipeFactory, m_userFeatureControl, m_mediaCopyWrapper);
    if (MOS_FAILED(status))
    {
        MOS_Delete(singlePipeCtx);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }
    else
    {
        m_vpPipeContexts.push_back(singlePipeCtx);
    }
    return status;
}

MOS_STATUS VpPipeline::CreateFeatureManager(VpResourceManager *vpResourceManager)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(m_reporting);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpMhwInterface.m_vpPlatformInterface);
    VP_PUBLIC_CHK_NULL_RETURN(vpResourceManager);

    // Add CheckFeatures api later in FeatureManagerNext.
    m_paramChecker = m_vpMhwInterface.m_vpPlatformInterface->CreateFeatureChecker(&m_vpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_paramChecker);

    m_vpInterface = MOS_New(VpInterface, &m_vpMhwInterface, *m_allocator, vpResourceManager);

    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface);

    m_featureManager = MOS_New(VpFeatureManagerNext, *m_vpInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

    VP_PUBLIC_CHK_STATUS_RETURN(((VpFeatureManagerNext *)m_featureManager)->Init(nullptr));

    m_pipelineParamFactory = MOS_New(VpPipelineParamFactory);
    VP_PUBLIC_CHK_NULL_RETURN(m_pipelineParamFactory);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CreateVpKernelSets()
{
    VP_FUNC_CALL();
    if (nullptr == m_kernelSet)
    {
        m_kernelSet = MOS_New(VpKernelSet, &m_vpMhwInterface, m_allocator);
        VP_PUBLIC_CHK_NULL_RETURN(m_kernelSet);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::CheckFeatures(void *params, bool &bapgFuncSupported)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(m_paramChecker);
    return m_paramChecker->CheckFeatures(params, bapgFuncSupported);
}

MOS_STATUS VpPipeline::CreateFeatureReport()
{
    VP_FUNC_CALL();

    if (m_vpMhwInterface.m_reporting)
    {
        if (m_reporting && m_reporting->owner == this && m_vpMhwInterface.m_reporting != m_reporting)
        {
            MOS_FreeMemory(m_reporting);
        }
        m_reporting = m_vpMhwInterface.m_reporting;
    }
    else
    {
        if (m_reporting == nullptr)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(CreateReport());
        }
        m_vpMhwInterface.m_reporting = m_reporting;
    }

    VP_PUBLIC_CHK_NULL_RETURN(m_reporting);
    return MOS_STATUS_SUCCESS;
}

#if (_DEBUG || _RELEASE_INTERNAL)
VPHAL_SURFACE *VpPipeline::AllocateTempTargetSurface(VPHAL_SURFACE *m_tempTargetSurface)
{
    VP_FUNC_CALL();

    m_tempTargetSurface = (VPHAL_SURFACE *)MOS_AllocAndZeroMemory(sizeof(VPHAL_SURFACE));
    if (!m_tempTargetSurface)
    {
        return nullptr;
    }
    return m_tempTargetSurface;
}

MOS_STATUS VpPipeline::SurfaceReplace(PVP_PIPELINE_PARAMS params)
{
    MOS_STATUS                  eStatus = MOS_STATUS_SUCCESS;
    bool                        allocated;

    MEDIA_FEATURE_TABLE *skuTable = nullptr;
    skuTable                      = m_vpMhwInterface.m_osInterface->pfnGetSkuTable(m_vpMhwInterface.m_osInterface);
    VP_PUBLIC_CHK_NULL_RETURN(skuTable);
    VP_PUBLIC_CHK_NULL_RETURN(m_userFeatureControl);

    if (m_userFeatureControl->EnabledSFCNv12P010LinearOutput() &&
        MOS_TILE_LINEAR != params->pTarget[0]->TileType &&
        (Format_P010 == params->pTarget[0]->Format || Format_NV12 == params->pTarget[0]->Format) &&
        MEDIA_IS_SKU(skuTable, FtrSFC420LinearOutputSupport))
    {
        if (!m_tempTargetSurface)
        {
            m_tempTargetSurface = AllocateTempTargetSurface(m_tempTargetSurface);
        }
        VP_PUBLIC_CHK_NULL_RETURN(m_tempTargetSurface);
        eStatus = m_allocator->ReAllocateSurface(
            m_tempTargetSurface,
            "TempTargetSurface",
            params->pTarget[0]->Format,
            MOS_GFXRES_2D,
            MOS_TILE_LINEAR,
            params->pTarget[0]->dwWidth,
            params->pTarget[0]->dwHeight,
            false,
            MOS_MMC_DISABLED,
            &allocated);

        m_tempTargetSurface->ColorSpace = params->pTarget[0]->ColorSpace;
        m_tempTargetSurface->rcSrc      = params->pTarget[0]->rcSrc;
        m_tempTargetSurface->rcDst      = params->pTarget[0]->rcDst;
        m_tempTargetSurface->rcMaxSrc   = params->pTarget[0]->rcMaxSrc;

        if (eStatus == MOS_STATUS_SUCCESS)
        {
            params->pTarget[0] = m_tempTargetSurface;    //params is the copy of pcRenderParams which will not cause the memleak,
        }
    }

    typedef struct _RGBFormatConfig
    {
        MOS_FORMAT      format;
        MOS_TILE_TYPE   tielType;
    } RGBFormatConfig;

    static const RGBFormatConfig rgbCfg[VP_RGB_OUTPUT_OVERRIDE_ID_MAX] = {
        {Format_Invalid, MOS_TILE_INVALID},
        {Format_RGBP, MOS_TILE_LINEAR},
        {Format_RGBP, MOS_TILE_Y},
        {Format_R8G8B8, MOS_TILE_LINEAR},
        {Format_BGRP, MOS_TILE_LINEAR},
        {Format_BGRP, MOS_TILE_Y}
    };

    if (m_userFeatureControl->EnabledSFCRGBPRGB24Output() == VP_RGB_OUTPUT_OVERRIDE_ID_INVALID ||
        m_userFeatureControl->EnabledSFCRGBPRGB24Output() >= VP_RGB_OUTPUT_OVERRIDE_ID_MAX)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (rgbCfg[m_userFeatureControl->EnabledSFCRGBPRGB24Output()].format != params->pTarget[0]->Format &&
        MEDIA_IS_SKU(skuTable, FtrSFCRGBPRGB24OutputSupport))
    {
        if (!m_tempTargetSurface)
        {
            m_tempTargetSurface = AllocateTempTargetSurface(m_tempTargetSurface);
        }
        VP_PUBLIC_CHK_NULL_RETURN(m_tempTargetSurface);
        eStatus = m_allocator->ReAllocateSurface(
            m_tempTargetSurface,
            "TempTargetSurface",
            rgbCfg[m_userFeatureControl->EnabledSFCRGBPRGB24Output()].format,
            MOS_GFXRES_2D,
            rgbCfg[m_userFeatureControl->EnabledSFCRGBPRGB24Output()].tielType,
            params->pTarget[0]->dwWidth,
            params->pTarget[0]->dwHeight,
            false,
            MOS_MMC_DISABLED,
            &allocated);

        m_tempTargetSurface->ColorSpace = params->pTarget[0]->ColorSpace;
        m_tempTargetSurface->rcSrc      = params->pTarget[0]->rcSrc;
        m_tempTargetSurface->rcDst      = params->pTarget[0]->rcDst;
        m_tempTargetSurface->rcMaxSrc   = params->pTarget[0]->rcMaxSrc;

        if (eStatus == MOS_STATUS_SUCCESS)
        {
            params->pTarget[0] = m_tempTargetSurface;  //params is the copy of pcRenderParams which will not cause the memleak,
        }
    }

    return eStatus;
}
#endif

MOS_STATUS VpPipeline::PrepareVpPipelineParams(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(params);

#if (_DEBUG || _RELEASE_INTERNAL)
    SurfaceReplace(params);    // replace output surface from Tile-Y to Linear
#endif

    if ((m_vpMhwInterface.m_osInterface != nullptr))
    {
        // Set the component info
        m_vpMhwInterface.m_osInterface->Component = params->Component;

        // Init component(DDI entry point) info for perf measurement
        m_vpMhwInterface.m_osInterface->pfnSetPerfTag(m_vpMhwInterface.m_osInterface, VPHAL_NONE);
    }

    PMOS_RESOURCE ppSource[VPHAL_MAX_SOURCES] = {nullptr};
    PMOS_RESOURCE ppTarget[VPHAL_MAX_TARGETS] = {nullptr};

    VP_PUBLIC_CHK_NULL_RETURN(params->pTarget[0]);
    VP_PUBLIC_CHK_NULL_RETURN(m_allocator);
    VP_PUBLIC_CHK_NULL_RETURN(m_featureManager);

    // Can be removed, as all info can be gotten during AllocateVpSurface.
    VPHAL_GET_SURFACE_INFO  info = {};
    for (uint32_t i = 0; i < params->uSrcCount; ++i)
    {
        MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));
        VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(
            params->pSrc[i],
            info));
    }

    MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));
    VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(
        params->pTarget[0],
        info));

    if (params->uSrcCount>0)
    {
        if (params->pSrc[0]->pBwdRef && params->pSrc[0]->uBwdRefCount > 0)
        {
            MOS_ZeroMemory(&info, sizeof(VPHAL_GET_SURFACE_INFO));

            VP_PUBLIC_CHK_STATUS_RETURN(m_allocator->GetSurfaceInfo(
                params->pSrc[0]->pBwdRef,
                info));
        }

        if (!RECT1_CONTAINS_RECT2(params->pSrc[0]->rcMaxSrc, params->pSrc[0]->rcSrc))
        {
            params->pSrc[0]->rcMaxSrc = params->pSrc[0]->rcSrc;
        }
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

    PrepareVpPipelineScalabilityParams(params);

    for (uint32_t uiIndex = 0; uiIndex < params->uSrcCount; uiIndex++)
    {
        // Add Procamp limitation before Render pass selected
        // Brightness[-100.0,100.0], Contrast & Saturation[0.0,10.0]
        PVPHAL_PROCAMP_PARAMS pProcampParams = params->pSrc[uiIndex]->pProcampParams;
        if (pProcampParams && pProcampParams->bEnabled)
        {
            pProcampParams->fBrightness = MOS_MIN(MOS_MAX(-100.0f, pProcampParams->fBrightness), 100.0f);
            pProcampParams->fContrast   = MOS_MIN(MOS_MAX(0.0f, pProcampParams->fContrast), 10.0f);
            pProcampParams->fSaturation = MOS_MIN(MOS_MAX(0.0f, pProcampParams->fSaturation), 10.0f);
        }
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  prepare execution params for vp scalability pipeline
//! \param  [in] params
//!         src and dst surface's width and height
//! \return MOS_STATUS
//!         MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpPipeline::PrepareVpPipelineScalabilityParams(uint32_t srcWidth, uint32_t srcHeight, uint32_t dstWidth, uint32_t dstHeight)
{
    VP_FUNC_CALL();
    VP_PUBLIC_NORMALMESSAGE("Reset m_numVebox %d -> %d", m_numVebox, m_numVeboxOriginal);
    m_numVebox = m_numVeboxOriginal;
    MT_LOG1(MT_VP_HAL_VEBOXNUM_RESET, MT_NORMAL, MT_VP_HAL_VEBOX_NUMBER, m_numVebox)

    // Disable vesfc scalability when reg key "Enable Vebox Scalability" was set to zero
    if (m_forceMultiplePipe == (MOS_SCALABILITY_ENABLE_MODE_USER_FORCE | MOS_SCALABILITY_ENABLE_MODE_FALSE))
    {
        m_numVebox = 1;
    }
    else
    {
        if (((srcWidth > m_scalability_threshWidth) &&
             (srcHeight > m_scalability_threshHeight)) ||
            ((dstWidth > m_scalability_threshWidth) &&
             (dstHeight > m_scalability_threshHeight)))
        {
            // Enable vesfc scalability only with 4k+ clips
        }
        else
        {
            // disable vesfc scalability with 4k- resolution clips if reg "Enable Vebox Scalability" was not set as true
            if (m_forceMultiplePipe != (MOS_SCALABILITY_ENABLE_MODE_USER_FORCE | MOS_SCALABILITY_ENABLE_MODE_DEFAULT))
            {
                m_numVebox = 1;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  prepare execution params for vp scalability pipeline
//! \param  [in] params
//!         Pointer to VP scalability pipeline params
//! \return MOS_STATUS
//!         MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpPipeline::PrepareVpPipelineScalabilityParams(VEBOX_SFC_PARAMS* params)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(params->input.surface);
    VP_PUBLIC_CHK_NULL_RETURN(params->output.surface);
    
    VP_PUBLIC_CHK_STATUS_RETURN(PrepareVpPipelineScalabilityParams(
        MOS_MIN(params->input.surface->dwWidth, (uint32_t)params->input.rcSrc.right),
        MOS_MIN(params->input.surface->dwHeight, (uint32_t)params->input.rcSrc.bottom),
        MOS_MIN(params->output.surface->dwWidth, (uint32_t)params->output.rcDst.right),
        MOS_MIN(params->output.surface->dwHeight, (uint32_t)params->output.rcDst.bottom)));

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  prepare execution params for vp scalability pipeline
//! \param  [in] params
//!         Pointer to VP scalability pipeline params
//! \return MOS_STATUS
//!         MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpPipeline::PrepareVpPipelineScalabilityParams(PVP_PIPELINE_PARAMS params)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(params);
    if (params->pSrc[0] == nullptr)
    {
        VP_PUBLIC_NORMALMESSAGE("No input will not need scalability! ");
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_NULL_RETURN(params->pTarget[0]);

    VP_PUBLIC_CHK_STATUS_RETURN(PrepareVpPipelineScalabilityParams(
        MOS_MIN(params->pSrc[0]->dwWidth, (uint32_t)params->pSrc[0]->rcSrc.right),
        MOS_MIN(params->pSrc[0]->dwHeight, (uint32_t)params->pSrc[0]->rcSrc.bottom),
        MOS_MIN(params->pTarget[0]->dwWidth, (uint32_t)params->pTarget[0]->rcSrc.right),
        MOS_MIN(params->pTarget[0]->dwHeight, (uint32_t)params->pTarget[0]->rcSrc.bottom)));

    // Disable DN when vesfc scalability was enabled for output mismatch issue
    if (IsMultiple())
    {
        if (params->pSrc[0]->pDenoiseParams)
        {
            params->pSrc[0]->pDenoiseParams->bAutoDetect   = false;
            params->pSrc[0]->pDenoiseParams->bEnableChroma = false;
            params->pSrc[0]->pDenoiseParams->bEnableLuma   = false;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::Prepare(void * params)
{
    MOS_STATUS eStatus = MOS_STATUS_UNKNOWN;

    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(m_userFeatureControl);

    m_pvpParams = *(VP_PARAMS *)params;

#if (_DEBUG || _RELEASE_INTERNAL)
    // INTER_FRAME_MEMORY_NINJA_END_COUNTER will be reported in UserFeatureReport() function which runs in Execute()
    ReportIFNCC(true);
#endif
    // Get Output Pipe for Features. It should be configured in ExecuteVpPipeline.
    if (m_vpPipeContexts.size() <= 0 || m_vpPipeContexts[0] == nullptr)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    m_vpPipeContexts[0]->InitializeOutputPipe();

    if (PIPELINE_PARAM_TYPE_LEGACY == m_pvpParams.type)
    {
        m_userFeatureControl->Update((PVP_PIPELINE_PARAMS)m_pvpParams.renderParams);
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
    else if (PIPELINE_PARAM_TYPE_MEDIA_SFC_INTERFACE == m_pvpParams.type)
    {
        VEBOX_SFC_PARAMS *sfcParams = m_pvpParams.sfcParams;
        VP_PUBLIC_CHK_NULL_RETURN(sfcParams);

        VP_PUBLIC_CHK_STATUS_RETURN(PrepareVpPipelineScalabilityParams(sfcParams));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpPipeline::Execute()
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(ExecuteVpPipeline())
    VP_PUBLIC_CHK_STATUS_RETURN(UserFeatureReport());

    bool veboxFeatureInuse = (m_vpPipeContexts.size() >= 1) && (m_vpPipeContexts[0]) && (m_vpPipeContexts[0]->IsVeboxInUse());
    if (m_packetSharedContext && m_packetSharedContext->isVeboxFirstFrame && veboxFeatureInuse)
    {
        m_packetSharedContext->isVeboxFirstFrame = false;
    }

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      VpSinglePipeContext                                         */
/****************************************************************************************************/

VpSinglePipeContext::VpSinglePipeContext()
{
}

VpSinglePipeContext::~VpSinglePipeContext()
{
    MOS_Delete(m_packetReuseMgr);
    MOS_Delete(m_resourceManager);
}

MOS_STATUS VpSinglePipeContext::Init(PMOS_INTERFACE osInterface, VpAllocator *allocator, VphalFeatureReport *reporting, vp::VpPlatformInterface *vpPlatformInterface, PacketPipeFactory *packetPipeFactory, VpUserFeatureControl *userFeatureControl, MediaCopyWrapper *mediaCopyWrapper)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(CreateResourceManager(osInterface, allocator, reporting, vpPlatformInterface, userFeatureControl, mediaCopyWrapper));
    VP_PUBLIC_CHK_NULL_RETURN(m_resourceManager);
    VP_PUBLIC_CHK_STATUS_RETURN(CreatePacketReuseManager(packetPipeFactory, userFeatureControl));
    VP_PUBLIC_CHK_NULL_RETURN(m_packetReuseMgr);

    return MOS_STATUS_SUCCESS;
}

//!
//! \brief  create reource manager
//! \return MOS_STATUS
//!         MOS_STATUS_SUCCESS if success, else fail reason
//!
MOS_STATUS VpSinglePipeContext::CreateResourceManager(PMOS_INTERFACE osInterface, VpAllocator *allocator, VphalFeatureReport *reporting, vp::VpPlatformInterface *vpPlatformInterface, vp::VpUserFeatureControl *userFeatureControl, MediaCopyWrapper *mediaCopyWrapper)
{
    VP_FUNC_CALL();
    if (nullptr == m_resourceManager)
    {
        m_resourceManager = MOS_New(VpResourceManager, *osInterface, *allocator, *reporting, *vpPlatformInterface, mediaCopyWrapper, userFeatureControl);
        VP_PUBLIC_CHK_NULL_RETURN(m_resourceManager);
    }
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpSinglePipeContext::CreatePacketReuseManager(PacketPipeFactory *pPacketPipeFactory, VpUserFeatureControl *userFeatureControl)
{
    VP_FUNC_CALL();
    if (nullptr == m_packetReuseMgr)
    {
        m_packetReuseMgr = NewVpPacketReuseManagerObj(pPacketPipeFactory, userFeatureControl);
        VP_PUBLIC_CHK_NULL_RETURN(m_packetReuseMgr);
        VP_PUBLIC_CHK_STATUS_RETURN(m_packetReuseMgr->RegisterFeatures());
    }
    return MOS_STATUS_SUCCESS;
}
