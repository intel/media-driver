/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_ai_filter.cpp
//! \brief    Defines the common interface for ai
//!           this file is for the base interface which is shared by all ai filter in driver.
//!

#include "vp_ai_filter.h"
#include "vp_ai_kernel_pipe.h"
#include "vp_render_cmd_packet.h"

namespace vp
{

void AI_KERNEL_PARAM::Init()
{
    kernelArgs.clear();
    kernelName.clear();
    threadWidth  = 0;
    threadHeight = 0;
    localWidth   = 0;
    localHeight  = 0;
    kernelStatefulSurfaces.clear();
}

void _RENDER_AI_PARAMS::Init()
{
    ai_kernelParams.clear();
    ai_kernelConfig = {};
}

VpAiFilter::VpAiFilter(PVP_MHWINTERFACE vpMhwInterface) : VpFilter(vpMhwInterface)
{
}

MOS_STATUS VpAiFilter::Init()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAiFilter::Prepare()
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAiFilter::Destroy()
{
    VP_FUNC_CALL();

    if (m_renderAiParams)
    {
        MOS_Delete(m_renderAiParams);
    }
    for (auto& featureAiKrnArgMap : m_swFiltersKrnArgs)
    {
        for (auto& multiLayerKrnArgMapHandle : featureAiKrnArgMap)
        {
            for (auto &singleLayerKrnArgMapHandle : multiLayerKrnArgMapHandle.second)
            {
                for (auto &krnArgHandle : singleLayerKrnArgMapHandle.second)
                {
                    KRN_ARG &krnArg = krnArgHandle.second;
                    MOS_FreeMemAndSetNull(krnArg.pData);
                }
                singleLayerKrnArgMapHandle.second.clear();
            }
            multiLayerKrnArgMapHandle.second.clear();
        }
        featureAiKrnArgMap.clear();
    }
    m_swFiltersKrnArgs.clear();
    
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAiFilter::SetExecuteEngineCaps(
    SwFilterPipe   *executingPipe,
    VP_EXECUTE_CAPS vpExecuteCaps)
{
    VP_FUNC_CALL();

    m_executingPipe = executingPipe;
    m_executeCaps   = vpExecuteCaps;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAiFilter::CalculateEngineParams()
{
    VP_FUNC_CALL();
    if (m_executeCaps.bRender)
    {
        // create a filter Param buffer
        if (!m_renderAiParams)
        {
            m_renderAiParams = (PRENDER_AI_PARAMS)MOS_New(RENDER_AI_PARAMS);

            if (m_renderAiParams == nullptr)
            {
                VP_PUBLIC_ASSERTMESSAGE("render ai Pamas buffer allocate failed, return nullpointer");
                return MOS_STATUS_NO_SPACE;
            }
        }
        else
        {
            m_renderAiParams->Init();
        }

        VP_PUBLIC_CHK_STATUS_RETURN(InitKrnParams(m_renderAiParams->ai_kernelParams, *m_executingPipe));

    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Wrong engine caps!");
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS VpAiFilter::InitKrnParams(AI_KERNEL_PARAMS &krnParams, SwFilterPipe &executingPipe)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface);
    VP_PUBLIC_CHK_NULL_RETURN(m_pvpMhwInterface->m_vpPlatformInterface);

    for (uint32_t i = 0; i < executingPipe.GetSurfaceCount(true); ++i)
    {
        SwFilterSubPipe *subPipe = executingPipe.GetSwFilterSubPipe(true, i);
        if (subPipe == nullptr)
        {
            continue;
        }
        while (m_swFiltersKrnArgs.size() <= i)
        {
            FEATURE_AI_KERNEL_ARG_MAP featureAiKrnArgMap = {};
            m_swFiltersKrnArgs.push_back(featureAiKrnArgMap);
        }
        FEATURE_AI_KERNEL_ARG_MAP &featureAiKrnArgMap = m_swFiltersKrnArgs.at(i);
        SwFilterAiBase            *ai                 = nullptr;
        VP_PUBLIC_CHK_STATUS_RETURN(subPipe->GetAiSwFilter(ai));
        if (ai == nullptr)
        {
            continue;
        }
        FeatureParamAi &swAiParam                 = ai->GetSwFilterParams();
        auto            multiLayerKrnArgMapHandle = featureAiKrnArgMap.find(ai->GetFeatureType());
        if (multiLayerKrnArgMapHandle == featureAiKrnArgMap.end())
        {
            MULTI_LAYERS_KERNEL_INDEX_ARG_MAP multiLayerKrnArgMap = {};
            multiLayerKrnArgMapHandle                             = featureAiKrnArgMap.insert(std::make_pair(ai->GetFeatureType(), multiLayerKrnArgMap)).first;
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(multiLayerKrnArgMapHandle, &featureAiKrnArgMap);
        }
        MULTI_LAYERS_KERNEL_INDEX_ARG_MAP &multiLayerKrnArgMap = multiLayerKrnArgMapHandle->second;
        
        uint32_t startIndex = (swAiParam.stageIndex == 0) ? 0 : ((swAiParam.stageIndex - 1) < swAiParam.kernelSplitGroupIndex.size() ? swAiParam.kernelSplitGroupIndex.at(swAiParam.stageIndex - 1) : 0);
        uint32_t endIndex    = swAiParam.stageIndex < swAiParam.kernelSplitGroupIndex.size() ? swAiParam.kernelSplitGroupIndex.at(swAiParam.stageIndex) : swAiParam.kernelSettings.size();
        if (startIndex >= endIndex)
        {
            VP_PUBLIC_ASSERTMESSAGE("Start Index of Kernel Pipe is Greater or Equal to End Index. The Execute Kernel Group will be Empty. Caused by Wrong Value in KernelSplitGroupIndex");
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        for (uint32_t layerIndex = startIndex; layerIndex < endIndex; ++layerIndex)
        {
            auto singleLayerKrnArgMapHandle = multiLayerKrnArgMap.find(layerIndex);
            if (singleLayerKrnArgMapHandle == multiLayerKrnArgMap.end())
            {
                KERNEL_INDEX_ARG_MAP singleLayerKrnArgMap = {};
                singleLayerKrnArgMapHandle                = multiLayerKrnArgMap.insert(std::make_pair(layerIndex, singleLayerKrnArgMap)).first;
                VP_PUBLIC_CHK_NOT_FOUND_RETURN(singleLayerKrnArgMapHandle, &multiLayerKrnArgMap);
            }
            if (layerIndex >= swAiParam.kernelSettings.size())
            {
                VP_PUBLIC_ASSERTMESSAGE("layer index %d is greater than the size of kernel pipe setting size. This is cause by setting wrong kernel split group index", layerIndex);
                VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
            AI_SINGLE_LAYER_SETTING &singleLayerSetting = swAiParam.kernelSettings.at(layerIndex);
            AI_KERNEL_PARAM          kernelParam        = {};
            kernelParam.Init();
            kernelParam.kernelName   = singleLayerSetting.kernelName;
            kernelParam.threadWidth  = singleLayerSetting.groupWidth;
            kernelParam.threadHeight = singleLayerSetting.groupHeight;
            kernelParam.localWidth   = singleLayerSetting.localWidth;
            kernelParam.localHeight  = singleLayerSetting.localHeight;

            auto handle = m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool().find(singleLayerSetting.kernelName);
            VP_PUBLIC_CHK_NOT_FOUND_RETURN(handle, &m_pvpMhwInterface->m_vpPlatformInterface->GetKernelPool());
            KERNEL_BTIS kernelBtis = handle->second.GetKernelBtis();
            KERNEL_ARGS kernelArgs = handle->second.GetKernelArgs();

            KERNEL_INDEX_ARG_MAP &singleLayerKrnArgMap = singleLayerKrnArgMapHandle->second;

            for (auto const &kernelArg : kernelArgs)
            {
                uint32_t argIndex     = kernelArg.uIndex;
                auto     krnArgHandle = singleLayerKrnArgMap.find(argIndex);
                if (krnArgHandle == singleLayerKrnArgMap.end())
                {
                    KRN_ARG krnArg = {};
                    krnArgHandle   = singleLayerKrnArgMap.insert(std::make_pair(argIndex, krnArg)).first;
                    VP_PUBLIC_CHK_NOT_FOUND_RETURN(krnArgHandle, &singleLayerKrnArgMap);
                }
                KRN_ARG &krnArg    = krnArgHandle->second;
                krnArg.uIndex      = kernelArg.uIndex;
                krnArg.eArgKind    = kernelArg.eArgKind;
                krnArg.uSize       = kernelArg.uSize;
                krnArg.addressMode = kernelArg.addressMode;
                bool bInit         = true;

                if (kernelArg.addressMode == AddressingModeStateless)
                {
                    if (krnArg.pData == nullptr)
                    {
                        krnArg.uSize = kernelArg.uSize;
                        krnArg.pData = MOS_AllocAndZeroMemory(sizeof(SURFACE_PARAMS));
                        VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
                    }
                    else
                    {
                        MOS_ZeroMemory(krnArg.pData, sizeof(SURFACE_PARAMS));
                    }
                    VP_PUBLIC_CHK_NULL_RETURN(singleLayerSetting.pfnSetStatelessSurface);
                    VP_PUBLIC_CHK_STATUS_RETURN(singleLayerSetting.pfnSetStatelessSurface(i, kernelArg.uIndex, executingPipe, *(PSURFACE_PARAMS)krnArg.pData, bInit));
                }
                else
                {
                    if (krnArg.pData == nullptr)
                    {
                        if (kernelArg.uSize > 0)
                        {
                            krnArg.uSize = kernelArg.uSize;
                            krnArg.pData = MOS_AllocAndZeroMemory(kernelArg.uSize);
                            VP_PUBLIC_CHK_NULL_RETURN(krnArg.pData);
                        }
                    }
                    else
                    {
                        VP_PUBLIC_CHK_VALUE_RETURN(krnArg.uSize, kernelArg.uSize);
                        MOS_ZeroMemory(krnArg.pData, krnArg.uSize);
                    }
                    VP_PUBLIC_CHK_NULL_RETURN(singleLayerSetting.pfnSetKernelArg);
                    VP_PUBLIC_CHK_STATUS_RETURN(singleLayerSetting.pfnSetKernelArg(i, kernelArg.uIndex, executingPipe, krnArg.pData, bInit));
                }

                if (bInit)
                {
                    kernelParam.kernelArgs.push_back(krnArg);
                }
            }

            for (auto const &kernelBti : kernelBtis)
            {
                uint32_t       uIndex       = kernelBti.first;
                SURFACE_PARAMS surfaceParam = {};
                bool           bInit        = true;
                VP_PUBLIC_CHK_NULL_RETURN(singleLayerSetting.pfnSetStatefulSurface);
                VP_PUBLIC_CHK_STATUS_RETURN(singleLayerSetting.pfnSetStatefulSurface(i, uIndex, executingPipe, surfaceParam, bInit));

                if (bInit)
                {
                    kernelParam.kernelStatefulSurfaces.insert(std::make_pair(uIndex, surfaceParam));
                }
            }
            krnParams.push_back(kernelParam);
        }
    }
    
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   HwFilter Ai Parameter                                          */
/****************************************************************************************************/
HwFilterParameter *HwFilterAiParameter::Create(HW_FILTER_AI_PARAM &param, FeatureType featureType)
{
    VP_FUNC_CALL();

    HwFilterAiParameter *p = MOS_New(HwFilterAiParameter, featureType);
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            MOS_Delete(p);
            return nullptr;
        }
    }
    return p;
}

HwFilterAiParameter::HwFilterAiParameter(FeatureType featureType) : HwFilterParameter(featureType)
{
}

HwFilterAiParameter::~HwFilterAiParameter()
{
}

MOS_STATUS HwFilterAiParameter::ConfigParams(HwFilter &hwFilter)
{
    VP_FUNC_CALL();

    return hwFilter.ConfigParam(m_Params);
}

MOS_STATUS HwFilterAiParameter::Initialize(HW_FILTER_AI_PARAM &param)
{
    VP_FUNC_CALL();

    m_Params = param;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Packet Ai Parameter                                       */
/****************************************************************************************************/
VpPacketParameter *VpRenderAiParameter::Create(HW_FILTER_AI_PARAM &param)
{
    VP_FUNC_CALL();

    if (nullptr == param.pPacketParamFactory)
    {
        return nullptr;
    }
    VpRenderAiParameter *p = dynamic_cast<VpRenderAiParameter *>(param.pPacketParamFactory->GetPacketParameter(param.pHwInterface));
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            VpPacketParameter *pParam = p;
            param.pPacketParamFactory->ReturnPacketParameter(pParam);
            return nullptr;
        }
    }
    return p;
}

VpRenderAiParameter::VpRenderAiParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory) : VpPacketParameter(packetParamFactory), m_filter(pHwInterface)
{
}
VpRenderAiParameter::~VpRenderAiParameter() {}

bool VpRenderAiParameter::SetPacketParam(VpCmdPacket *pPacket)
{
    VP_FUNC_CALL();

    VpRenderCmdPacket *renderPacket = dynamic_cast<VpRenderCmdPacket *>(pPacket);
    if (nullptr == renderPacket)
    {
        return false;
    }

    PRENDER_AI_PARAMS params = m_filter.GetAiParams();
    if (nullptr == params)
    {
        return false;
    }
    return MOS_SUCCEEDED(renderPacket->SetAiParams(params));
}

MOS_STATUS VpRenderAiParameter::Initialize(HW_FILTER_AI_PARAM &params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(m_filter.Init());
    VP_PUBLIC_CHK_STATUS_RETURN(m_filter.SetExecuteEngineCaps(params.executingPipe, params.vpExecuteCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(m_filter.CalculateEngineParams());
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                   Policy Ai Handler                                              */
/****************************************************************************************************/
PolicyAiHandler::PolicyAiHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
{
    m_Type = FeatureTypeAiOnRender;
}
PolicyAiHandler::~PolicyAiHandler()
{
}

bool PolicyAiHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return vpExecuteCaps.bAiPath;
}

HwFilterParameter *PolicyAiHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    VP_FUNC_CALL();

    if (IsFeatureEnabled(vpExecuteCaps))
    {
        HW_FILTER_AI_PARAM param = {};
        param.type                   = m_Type;
        param.pHwInterface           = pHwInterface;
        param.vpExecuteCaps          = vpExecuteCaps;
        param.pPacketParamFactory    = &m_PacketParamFactory;
        param.executingPipe          = &swFilterPipe;
        param.pfnCreatePacketParam   = PolicyAiHandler::CreatePacketParam;

        HwFilterParameter *pHwFilterParam = GetHwFeatureParameterFromPool();

        if (pHwFilterParam)
        {
            if (MOS_FAILED(((HwFilterAiParameter *)pHwFilterParam)->Initialize(param)))
            {
                ReleaseHwFeatureParameter(pHwFilterParam);
            }
        }
        else
        {
            pHwFilterParam = HwFilterAiParameter::Create(param, m_Type);
        }

        return pHwFilterParam;
    }
    else
    {
        return nullptr;
    }
}

MOS_STATUS PolicyAiHandler::UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    SwFilterAiBase *featureAI = dynamic_cast<SwFilterAiBase *>(&feature);
    VP_PUBLIC_CHK_NULL_RETURN(featureAI);
    FeatureParamAi &param = featureAI->GetSwFilterParams();

    if (caps.bAiPath && param.stageIndex < param.kernelSplitGroupIndex.size())
    {
        SwFilterAiBase *filter2ndPass = featureAI;
        VP_PUBLIC_CHK_NULL_RETURN(filter2ndPass);

        FeatureParamAi &params2ndPass = filter2ndPass->GetSwFilterParams();
        SwFilterAiBase *filter1ndPass = (SwFilterAiBase *)feature.Clone();
        VP_PUBLIC_CHK_NULL_RETURN(filter1ndPass);
        filter1ndPass->GetFilterEngineCaps() = filter2ndPass->GetFilterEngineCaps();
        filter1ndPass->SetFeatureType(filter2ndPass->GetFeatureType());
        FeatureParamAi &params1stPass = filter1ndPass->GetSwFilterParams();

        params2ndPass.stageIndex += 1;
        // Clear engine caps for filter in 2nd pass
        filter2ndPass->SetFeatureType(FeatureType(filter2ndPass->GetFeatureType() & FEATURE_TYPE_MASK));
        filter2ndPass->SetRenderTargetType(RenderTargetTypeSurface);
        filter2ndPass->GetFilterEngineCaps().value           = 0;
        filter2ndPass->GetFilterEngineCaps().bEnabled        = 1;
        filter2ndPass->GetFilterEngineCaps().RenderNeeded    = 1;
        filter2ndPass->GetFilterEngineCaps().isolated        = 1;
        filter2ndPass->GetFilterEngineCaps().multiPassNeeded = params2ndPass.stageIndex < params2ndPass.kernelSplitGroupIndex.size();
        if (params2ndPass.stageIndex >= param.kernelSplitGroupIndex.size())
        {
            filter2ndPass->GetFilterEngineCaps().isOutputPipeNeeded = true;
        }
        
        executePipe.AddSwFilterUnordered(filter1ndPass, isInputPipe, index);
    }
    else
    {
        return PolicyFeatureHandler::UpdateFeaturePipe(caps, feature, featurePipe, executePipe, isInputPipe, index);
    }

    return MOS_STATUS_SUCCESS;
}

}