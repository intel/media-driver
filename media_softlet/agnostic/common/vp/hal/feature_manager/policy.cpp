/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     policy.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "policy.h"
#include "vp_obj_factories.h"
#include "vp_feature_manager.h"
#include "vp_platform_interface.h"
#include "sw_filter_handle.h"
#include "vp_cgc_filter.h"
#include "vp_user_feature_control.h"


namespace vp
{
/****************************************************************************************************/
/*                                      Policy                                                      */
/****************************************************************************************************/

Policy::Policy(VpInterface &vpInterface) : m_vpInterface(vpInterface)
{
}

Policy::~Policy()
{
    UnregisterFeatures();
}

MOS_STATUS Policy::UpdateVpHwCapsBasedOnSku(VP_HW_CAPS &vpHwCaps)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::Initialize()
{
    VP_FUNC_CALL();

    VpPlatformInterface *vpPlatformInterface = (VpPlatformInterface *)m_vpInterface.GetHwInterface()->m_vpPlatformInterface;
    VP_PUBLIC_CHK_NULL_RETURN(vpPlatformInterface);
    VP_PUBLIC_CHK_STATUS_RETURN(vpPlatformInterface->InitVpHwCaps(m_hwCaps));
    //update caps
    UpdateVpHwCapsBasedOnSku(m_hwCaps);
    VP_PUBLIC_CHK_STATUS_RETURN(RegisterFeatures());
    m_initialized = true;
    return MOS_STATUS_SUCCESS;
}

bool Policy::IsVeboxSfcFormatSupported(MOS_FORMAT  formatInput, MOS_FORMAT formatOutput)
{
    VP_FUNC_CALL();

    if (!m_initialized)
    {
        return false;
    }
    if (m_hwCaps.m_sfcHwEntry[formatInput].inputSupported   &&
        m_hwCaps.m_sfcHwEntry[formatOutput].outputSupported)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool Policy::IsAlphaEnabled(FeatureParamScaling* scalingParams)
{
    VP_FUNC_CALL();

    if (scalingParams == nullptr)
    {
        return false;
    }

    return scalingParams->pCompAlpha != nullptr;
}

bool Policy::IsColorfillEnabled(FeatureParamScaling* scalingParams)
{
    VP_FUNC_CALL();

    if (scalingParams == nullptr)
    {
        return false;
    }

    bool isColorFill = false;
    isColorFill      = (scalingParams->pColorFillParams &&
                     (!scalingParams->pColorFillParams->bDisableColorfillinSFC) &&
                     (scalingParams->pColorFillParams->bOnePixelBiasinSFC ? 
                     (!RECT1_CONTAINS_RECT2_ONEPIXELBIAS(scalingParams->input.rcDst, scalingParams->output.rcDst)):
                     (!RECT1_CONTAINS_RECT2(scalingParams->input.rcDst, scalingParams->output.rcDst))))
                     ? true
                     : false;

    return isColorFill;
}

MOS_STATUS Policy::RegisterFeatures()
{
    VP_FUNC_CALL();

    UnregisterFeatures();

    // Vebox/Sfc features.
    PolicyFeatureHandler *p = MOS_New(PolicySfcCscHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeCscOnSfc, p));

    p = MOS_New(PolicySfcRotMirHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeRotMirOnSfc, p));

    p = MOS_New(PolicySfcScalingHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeScalingOnSfc, p));

    p = MOS_New(PolicyVeboxDnHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeDnOnVebox, p));

    p = MOS_New(PolicyRenderDnHVSCalHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeDnHVSCalOnRender, p));

    p = MOS_New(PolicyVeboxCscHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeCscOnVebox, p));

    p = MOS_New(PolicyVeboxSteHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeSteOnVebox, p));

    p = MOS_New(PolicyVeboxTccHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeTccOnVebox, p));

    p = MOS_New(PolicyVeboxProcampHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeProcampOnVebox, p));

    p = MOS_New(PolicyVeboxHdrHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeHdrOnVebox, p));

    p = MOS_New(PolicyRenderHdr3DLutCalHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeHdr3DLutCalOnRender, p));

    p = MOS_New(PolicyRenderHdrHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeHdrOnRender, p));

    p = MOS_New(PolicyDiHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeDiOnVebox, p));

    p = MOS_New(PolicySfcColorFillHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeColorFillOnSfc, p));

    p = MOS_New(PolicySfcAlphaHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeAlphaOnSfc, p));

    VP_PUBLIC_CHK_STATUS_RETURN(RegisterFcFeatures());

    p = MOS_New(PolicyVeboxCgcHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeCgcOnVebox, p));

    // Next step to add a table to trace all SW features based on platforms
    m_featurePool.push_back(FeatureTypeCsc);
    m_featurePool.push_back(FeatureTypeScaling);
    m_featurePool.push_back(FeatureTypeRotMir);
    m_featurePool.push_back(FeatureTypeDn);
    m_featurePool.push_back(FeatureTypeSte);
    m_featurePool.push_back(FeatureTypeTcc);
    m_featurePool.push_back(FeatureTypeProcamp);
    m_featurePool.push_back(FeatureTypeHdr);
    m_featurePool.push_back(FeatureTypeDi);
    m_featurePool.push_back(FeatureTypeFc);
    m_featurePool.push_back(FeatureTypeLumakey);
    m_featurePool.push_back(FeatureTypeBlending);
    m_featurePool.push_back(FeatureTypeColorFill);
    m_featurePool.push_back(FeatureTypeAlpha);
    m_featurePool.push_back(FeatureTypeCgc);

    return MOS_STATUS_SUCCESS;
}

void Policy::UnregisterFeatures()
{
    while (!m_VeboxSfcFeatureHandlers.empty())
    {
        std::map<FeatureType, PolicyFeatureHandler*>::iterator it = m_VeboxSfcFeatureHandlers.begin();
        MOS_Delete(it->second);
        m_VeboxSfcFeatureHandlers.erase(it);
    }

    while (!m_RenderFeatureHandlers.empty())
    {
        std::map<FeatureType, PolicyFeatureHandler*>::iterator it = m_RenderFeatureHandlers.begin();
        MOS_Delete(it->second);
        m_RenderFeatureHandlers.erase(it);
    }

    m_featurePool.clear();
}

MOS_STATUS Policy::RegisterFcFeatures()
{
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface())
    VpUserFeatureControl *vpUserFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    VP_PUBLIC_CHK_NULL_RETURN(vpUserFeatureControl);
    bool enableOclFC = vpUserFeatureControl->EnableOclFC();

    //Legacy Fc and OCL FC switching will be done in the wrapper handler class
    //It will use Legacy FC if vpUserFeatureControl->EnableOclFC() is not true, which means specific platform not support OCL FC
    //It will fall back to legacy FC when vp execute caps show bLegacyFC is true, which means some formats OCL FC not supported yet
    //In the future, after all caps formats added for OCL FC, the wrapper will be removed and only register specific OCL/Legacy FC handler
    PolicyFeatureHandler *p = MOS_New(PolicyFcWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeFcOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeLumakeyOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeBlendingOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeColorFillOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeAlphaOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeCscOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeScalingOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeRotMirOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeDiOnRender, p));

    p = MOS_New(PolicyFcFeatureWrapHandler, m_hwCaps, enableOclFC);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeProcampOnRender, p));

#if (_DEBUG || _RELEASE_INTERNAL)
    VpFeatureReport *vpFeatureReport = m_vpInterface.GetHwInterface()->m_reporting;
    if (vpFeatureReport)
    {
        vpFeatureReport->GetFeatures().isOclFC = enableOclFC;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

/*                                    Enable SwFilterPipe                                           */

MOS_STATUS Policy::CreateHwFilter(SwFilterPipe &subSwFilterPipe, HwFilter *&pFilter)
{
    VP_FUNC_CALL();

    if (subSwFilterPipe.IsEmpty())
    {
        pFilter = nullptr;
        return MOS_STATUS_SUCCESS;
    }

    HW_FILTER_PARAMS param = {};

    MT_LOG(MT_VP_FEATURE_GRAPH_SETUPEXECUTESWFILTER_START, MT_NORMAL);
    VP_PUBLIC_NORMALMESSAGE("Feature Graph: Setup ExecutePipe Start");
    MOS_STATUS status = GetHwFilterParam(subSwFilterPipe, param);

    if (MOS_FAILED(status))
    {
        VP_PUBLIC_ASSERTMESSAGE("Create HW Filter Failed, Return Error");
        MT_ERR2(MT_VP_HAL_POLICY, MT_ERROR_CODE, status, MT_CODE_LINE, __LINE__);
        return status;
    }

    pFilter = m_vpInterface.GetHwFilterFactory().Create(param);

    ReleaseHwFilterParam(param);

    if (!pFilter)
    {
        VP_PUBLIC_ASSERTMESSAGE("Create HW Filter Failed, Return Error");
        MT_ERR2(MT_VP_HAL_POLICY, MT_ERROR_CODE, MOS_STATUS_UNIMPLEMENTED, MT_CODE_LINE, __LINE__);
        if (m_vpInterface.GetHwInterface()->m_userFeatureControl->Is3DLutKernelOnly())
        {
            VP_PUBLIC_NORMALMESSAGE("Bypass This Workload due to 3dlut kenrel test, Return True");
            return MOS_STATUS_SUCCESS;
        }
        return MOS_STATUS_UNIMPLEMENTED;
    }
    MT_LOG(MT_VP_FEATURE_GRAPH_SETUPEXECUTESWFILTER_END, MT_NORMAL);
    VP_PUBLIC_NORMALMESSAGE("Feature Graph: Setup ExecutePipe End");
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetHwFilterParam(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    MOS_STATUS status = MOS_STATUS_SUCCESS;

    params.Type = EngineTypeInvalid;

    // Create and clear executedFilters.
    if (params.executedFilters)
    {
        params.executedFilters->Clean();
    }
    else
    {
        status = m_vpInterface.GetSwFilterPipeFactory().Create(params.executedFilters);

        if (status != MOS_STATUS_SUCCESS)
        {
            m_vpInterface.GetSwFilterPipeFactory().Destory(params.executedFilters);
            VP_PUBLIC_ASSERTMESSAGE("Create Executed Filter Failed, Return Error");
            MT_ERR2(MT_VP_HAL_POLICY, MT_ERROR_CODE, status, MT_CODE_LINE, __LINE__);
            return status;
        }
    }

    params.executedFilters->SetExePipeFlag(true);

    status = GetExecuteCaps(subSwFilterPipe, params);

    if (status != MOS_STATUS_SUCCESS)
    {
        m_vpInterface.GetSwFilterPipeFactory().Destory(params.executedFilters);
        VP_PUBLIC_ASSERTMESSAGE("Create Executed Filter Failed, Return Error");
        MT_ERR2(MT_VP_HAL_POLICY, MT_ERROR_CODE, status, MT_CODE_LINE, __LINE__);
        return status;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecuteCaps(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_EngineEntry   engineCapsCombinedAllPipes = {};
    SwFilterSubPipe* pipe = nullptr;
    uint32_t index = 0;
    uint32_t inputSurfCount         = subSwFilterPipe.GetSurfaceCount(true);
    uint32_t outputSurfCount        = subSwFilterPipe.GetSurfaceCount(false);

    VP_PUBLIC_NORMALMESSAGE("Only Support primary layer for advanced processing");

    engineCapsCombinedAllPipes.value = 0;

    for (index = 0; index < inputSurfCount; ++index)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(BuildExecutionEngines(subSwFilterPipe, true, index, engineCapsCombinedAllPipes));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateExecuteEngineCapsForCrossPipeFeatures(subSwFilterPipe, engineCapsCombinedAllPipes));

    for (index = 0; index < outputSurfCount; ++index)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(BuildExecutionEngines(subSwFilterPipe, false, index, engineCapsCombinedAllPipes));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(BuildFilters(subSwFilterPipe, params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecutionCapsForSingleFeature(FeatureType featureType, SwFilterSubPipe& swFilterPipe, VP_EngineEntry& engineCapsCombined)
{
    VP_FUNC_CALL();

    SwFilter* feature = swFilterPipe.GetSwFilter(featureType);
    SwFilter* di      = nullptr;
    SwFilter* hdr     = nullptr;
    SwFilter* cgc     = nullptr;
    SwFilter* scaling = nullptr;
    VPHAL_CSPACE cscInputColorSpaceCheck = CSpace_None;

    auto getDeinterlaceExecutionCaps = [&](SwFilter *_di)
    {
        bool forceDIToRender = false;
        VP_PUBLIC_CHK_NULL_RETURN(_di);
        scaling = scaling ? scaling : dynamic_cast<SwFilterScaling*>(swFilterPipe.GetSwFilter(FeatureType(FeatureTypeScaling)));
        if (scaling)
        {
            // No need check HDR for DI case.
            VP_PUBLIC_CHK_STATUS_RETURN(GetScalingExecutionCaps(scaling, false, true));
            forceDIToRender = scaling->GetFilterEngineCaps().RenderNeeded && scaling->GetFilterEngineCaps().sfcNotSupported;
        }
        return GetDeinterlaceExecutionCaps(_di, forceDIToRender);
    };

    if (!feature)
    {
        VP_PUBLIC_VERBOSEMESSAGE("Feature %d is not enabled in current pipe", featureType);
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        VP_PUBLIC_NORMALMESSAGE("Feature %d is enabled in current pipe", featureType);
    }

    // Always clean engine caps before Query. For next step, need to add new caps variable in swFilter
    // for forcing engine case.
    // feature->GetFilterEngineCaps().value = 0;

    switch (featureType)
    {
    case FeatureTypeCsc:
        hdr = swFilterPipe.GetSwFilter(FeatureTypeHdr);
        di  = swFilterPipe.GetSwFilter(FeatureTypeDi);
        cgc = dynamic_cast<SwFilterCgc *>(swFilterPipe.GetSwFilter(FeatureType(FeatureTypeCgc)));
        cscInputColorSpaceCheck = (&((SwFilterCsc *)feature)->GetSwFilterParams())->input.colorSpace;
        if (hdr)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCapsHdr(hdr, feature));
        }
        else if (IS_COLOR_SPACE_BT2020_YUV(cscInputColorSpaceCheck) && cgc)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCapsBT2020ToRGB(cgc, feature));
        }
        else if (di)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(getDeinterlaceExecutionCaps(di));

            VP_EngineEntry *diEngine = &di->GetFilterEngineCaps();
            if (diEngine->bEnabled && diEngine->VeboxNeeded)
            {
                VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCapsDi(feature));
            }
            else
            {
                VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCaps(feature, false));
            }
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCaps(feature, false));
        }
        break;
    case FeatureTypeScaling:
        hdr = dynamic_cast<SwFilterHdr *>(swFilterPipe.GetSwFilter(FeatureType(FeatureTypeHdr)));
        if (hdr)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(GetScalingExecutionCapsHdr(feature));
        }
        else
        {
            di = swFilterPipe.GetSwFilter(FeatureTypeDi);
            VP_PUBLIC_CHK_STATUS_RETURN(GetScalingExecutionCaps(feature, di != nullptr));
        }
        break;
    case FeatureTypeRotMir:
        VP_PUBLIC_CHK_STATUS_RETURN(GetRotationExecutionCaps(feature));
        break;
    case FeatureTypeDn:
        VP_PUBLIC_CHK_STATUS_RETURN(GetDenoiseExecutionCaps(feature));
        break;
    case FeatureTypeSte:
        VP_PUBLIC_CHK_STATUS_RETURN(GetSteExecutionCaps(feature));
        break;
    case FeatureTypeTcc:
        VP_PUBLIC_CHK_STATUS_RETURN(GetTccExecutionCaps(feature));
        break;
    case FeatureTypeProcamp:
        VP_PUBLIC_CHK_STATUS_RETURN(GetProcampExecutionCaps(feature));
        break;
    case FeatureTypeHdr:
        VP_PUBLIC_CHK_STATUS_RETURN(GetHdrExecutionCaps(feature));
        break;
    case FeatureTypeDi:
        VP_PUBLIC_CHK_STATUS_RETURN(getDeinterlaceExecutionCaps(feature));
        break;
    case FeatureTypeLumakey:
        VP_PUBLIC_CHK_STATUS_RETURN(GetLumakeyExecutionCaps(feature));
        break;
    case FeatureTypeBlending:
        VP_PUBLIC_CHK_STATUS_RETURN(GetBlendingExecutionCaps(feature));
        break;
    case FeatureTypeColorFill:
        VP_PUBLIC_CHK_STATUS_RETURN(GetColorFillExecutionCaps(feature));
        break;
    case FeatureTypeAlpha:
        VP_PUBLIC_CHK_STATUS_RETURN(GetAlphaExecutionCaps(feature));
        break;
    case FeatureTypeCgc:
        VP_PUBLIC_CHK_STATUS_RETURN(GetCgcExecutionCaps(feature));
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(GetExecutionCaps(feature));
        VP_PUBLIC_ASSERTMESSAGE("Feature is not supported by driver, bypass it");
        break;
    }

    VP_EngineEntry& engineCaps = feature->GetFilterEngineCaps();

    if (engineCaps.value == 0)
    {
        // Return success after all feature enabled and fully switched to APO path.
        VP_PUBLIC_NORMALMESSAGE("No engine being assigned for feature %d. Will bypass it.", featureType);
    }

    engineCapsCombined.value |= engineCaps.value;

    MT_LOG7(MT_VP_HAL_POLICY_GET_EXTCAPS4FTR, MT_NORMAL, MT_VP_HAL_FEATUERTYPE, featureType, MT_VP_HAL_ENGINECAPS, int64_t(engineCaps.value),
        MT_VP_HAL_ENGINECAPS_EN, int64_t(engineCaps.bEnabled), MT_VP_HAL_ENGINECAPS_VE_NEEDED, int64_t(engineCaps.VeboxNeeded),
        MT_VP_HAL_ENGINECAPS_SFC_NEEDED, int64_t(engineCaps.SfcNeeded), MT_VP_HAL_ENGINECAPS_RENDER_NEEDED, int64_t(engineCaps.RenderNeeded),
        MT_VP_HAL_ENGINECAPS_FC_SUPPORT, int64_t(engineCaps.fcSupported));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateExecuteEngineCapsForHDR(SwFilterPipe &swFilterPipe, VP_EngineEntry &engineCapsCombinedAllPipes)
{
    VP_FUNC_CALL();

    uint32_t pipeCount = swFilterPipe.GetSurfaceCount(true);
    std::vector<SwFilterHdr *> hdrFilters;
    bool isHdrKernelNeeded = false;
    auto outputSurf = swFilterPipe.GetSurface(false, 0);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);

    if (1 == pipeCount)
    {
        return MOS_STATUS_SUCCESS;
    }

    // Multi-layer S2H or H2H case.
    if (pipeCount > 1 && IS_COLOR_SPACE_BT2020(outputSurf->ColorSpace))
    {
        VP_PUBLIC_NORMALMESSAGE("multi-layer H2H or S2H, use HDR kernel.");
        isHdrKernelNeeded = true;
    }

    for (uint32_t i = 0; i < pipeCount; ++i)
    {
        auto filter = (SwFilterHdr *)swFilterPipe.GetSwFilter(true, i, FeatureTypeHdr);
        if (nullptr == filter)
        {
            continue;
        }

        if (!filter->GetFilterEngineCaps().bEnabled)
        {
            continue;
        }  

        hdrFilters.push_back(filter);
    }

    if (!isHdrKernelNeeded)
    {
        // 1 H2S layer, use vebox 3DLut + fc or HDR kernel based on hdrKernelNeeded flag on hdr filter.
        if (hdrFilters.size() <= 1)
        {
            return MOS_STATUS_SUCCESS;
        }
        VP_PUBLIC_NORMALMESSAGE("multi-layer H2S, use HDR kernel.");
        isHdrKernelNeeded = true;
    }

    if (isHdrKernelNeeded)
    {
        for (auto filter : hdrFilters)
        {
            auto &engineCaps = filter->GetFilterEngineCaps();
            if (!filter->GetFilterEngineCaps().hdrKernelNeeded)
            {
                engineCaps.VeboxNeeded          = 0;
                engineCaps.RenderNeeded         = 1;
                engineCaps.hdrKernelNeeded      = 1;
                engineCaps.hdrKernelSupported   = 1;
                VP_PUBLIC_NORMALMESSAGE("Update HDR filter to use HDR kernel.");
                PrintFeatureExecutionCaps(__FUNCTION__, engineCaps);
            }
        }
        engineCapsCombinedAllPipes.RenderNeeded         = 1;
        engineCapsCombinedAllPipes.hdrKernelNeeded      = 1;
        engineCapsCombinedAllPipes.hdrKernelSupported   = 1;

        for (uint32_t i = 0; i < pipeCount; ++i)
        {
            SwFilterSubPipe *pipe = nullptr;
            pipe                  = swFilterPipe.GetSwFilterSubPipe(true, i);
            // All layers will be processed by hdr kernel if hdr kernel being used. The features not supported by hdr kernel in all pipes will be filtered out.
            VP_PUBLIC_CHK_STATUS_RETURN(FilterFeatureCombinationForHDRKernel(pipe));

        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateExecuteEngineCapsForCrossPipeFeatures(SwFilterPipe &swFilterPipe, VP_EngineEntry &engineCapsCombinedAllPipes)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateExecuteEngineCapsForHDR(swFilterPipe, engineCapsCombinedAllPipes));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecutionEngines(SwFilterPipe &swFilterPipe, bool isInputPipe, uint32_t index, VP_EngineEntry &engineCapsCombinedAllPipes)
{
    VP_FUNC_CALL();

    SwFilterSubPipe *pipe    = nullptr;
    SwFilter *       feature = nullptr;
    pipe                     = swFilterPipe.GetSwFilterSubPipe(isInputPipe, index);
    VP_EngineEntry engineCapsCombined = {};

    engineCapsCombined.value = 0;

    VP_PUBLIC_NORMALMESSAGE("isInputPipe %d, index %d", (isInputPipe ? 1 : 0), index);

    if (pipe)
    {
        for (auto filterID : m_featurePool)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(GetExecutionCapsForSingleFeature(filterID, *pipe, engineCapsCombined));
        }
        engineCapsCombinedAllPipes.value |= engineCapsCombined.value;
        VP_PUBLIC_CHK_STATUS_RETURN(FilterFeatureCombination(swFilterPipe, isInputPipe, index, engineCapsCombined, engineCapsCombinedAllPipes));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::Update3DLutoutputColorAndFormat(FeatureParamCsc *cscParams, FeatureParamHdr *hdrParams, MOS_FORMAT Format, VPHAL_CSPACE CSpace)
{
    // For vebox + render, e.g. BT2020 P010->SRGB, if not correct the format here, since forceCscToRender being enabled, outputFormat in csc filter of
    // first pass will be set to P010 in PolicyVeboxCscHandler::UpdateFeaturePipe(). It will cause the wrong format being used for intermediate surface
    // in VpResourceManager::GetIntermediaOutputSurfaceColorAndFormat().
    // For vebox + sfc, GetSfcInputFormat in VpCscFilter::CalculateSfcEngineParams() will ensure the right format being used, although the format
    // in csc will be overwritten in params.executedFilters->Update() in Policy::BuildExecuteFilter.
    hdrParams->formatOutput     = Format;
    hdrParams->dstColorSpace    = CSpace;
    cscParams->formatInput      = Format;
    cscParams->input.colorSpace = CSpace;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetCSCExecutionCapsHdr(SwFilter *HDR, SwFilter *CSC)
{
    VP_FUNC_CALL();

    SwFilterHdr     *hdr       = nullptr;
    SwFilterCsc     *csc       = nullptr;
    FeatureParamHdr *hdrParams = nullptr;
    FeatureParamCsc *cscParams = nullptr;
    VP_EngineEntry  *cscEngine = nullptr;

    VP_PUBLIC_CHK_NULL_RETURN(HDR);
    VP_PUBLIC_CHK_NULL_RETURN(CSC);
    hdr = (SwFilterHdr *)HDR;
    csc = (SwFilterCsc *)CSC;

    hdrParams = &hdr->GetSwFilterParams();
    cscParams = &csc->GetSwFilterParams();
    cscEngine = &csc->GetFilterEngineCaps();

    // Clean usedForNextPass flag.
    if (cscEngine->usedForNextPass)
    {
        cscEngine->usedForNextPass = false;
    }
    if (cscEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("CSC Feature Already been processed, Skip further process");

        PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
        return MOS_STATUS_SUCCESS;
    }

    MOS_FORMAT   hdrFormat  = Format_Any;
    VPHAL_CSPACE hdrCSpace  = CSpace_Any;
    if (cscParams->isFullRgbG10P709 && IS_RGB64_FLOAT_FORMAT(cscParams->formatOutput))
    {
        hdrCSpace = CSpace_BT2020_RGB;
        hdrFormat = Format_A16B16G16R16;
    }
    else
    {
        hdrCSpace = IS_COLOR_SPACE_BT2020(cscParams->output.colorSpace) ? CSpace_BT2020_RGB : CSpace_sRGB;
        hdrFormat = IS_COLOR_SPACE_BT2020(cscParams->output.colorSpace) ? Format_R10G10B10A2 : Format_A8R8G8B8;
    }
    VP_PUBLIC_CHK_STATUS_RETURN(Update3DLutoutputColorAndFormat(cscParams, hdrParams, hdrFormat, hdrCSpace));

    if (m_hwCaps.m_sfcHwEntry[hdrFormat].inputSupported &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
        m_hwCaps.m_sfcHwEntry[hdrFormat].cscSupported)
    {
        if (hdrFormat == cscParams->formatOutput && hdrCSpace == cscParams->output.colorSpace)
        {
            VP_PUBLIC_NORMALMESSAGE("Skip CSC for HDR case.");
            cscEngine->forceEnableForSfc = 1;
            cscEngine->forceEnableForHdrKernel = 1;
        }
        else
        {
            cscEngine->bEnabled     = 1;
            cscEngine->SfcNeeded    = 1;
            cscEngine->RenderNeeded = 1;
            cscEngine->fcSupported  = 1;
            cscEngine->hdrKernelSupported = 1;
        }
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Post CSC for HDR not supported by SFC");
        return MOS_STATUS_INVALID_PARAMETER;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetCSCExecutionCapsDi(SwFilter* feature)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterCsc* csc = dynamic_cast<SwFilterCsc*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(csc);
    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableSfc         = userFeatureControl->IsSfcDisabled();

    VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCaps(feature, false));

    VP_EngineEntry *cscEngine = &csc->GetFilterEngineCaps();
    VP_PUBLIC_CHK_NULL_RETURN(cscEngine);
    if (!disableSfc && (cscEngine->bEnabled && (cscEngine->SfcNeeded || cscEngine->VeboxNeeded)) ||
        !cscEngine->bEnabled && cscEngine->forceEnableForSfc)
    {
        cscEngine->bEnabled     = 1;
        cscEngine->SfcNeeded    = 1;
        cscEngine->VeboxNeeded  = 0;
        cscEngine->RenderNeeded = 0;
    }
    else
    {
        cscEngine->bEnabled     = 1;
        cscEngine->SfcNeeded    = 0;
        cscEngine->VeboxNeeded  = 0;
        cscEngine->RenderNeeded = 1;
        cscEngine->fcSupported  = 1;
    }

    PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
    return MOS_STATUS_SUCCESS;
}


MOS_STATUS Policy::GetCSCExecutionCapsBT2020ToRGB(SwFilter *cgc, SwFilter *csc)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(csc);
    VP_PUBLIC_CHK_NULL_RETURN(cgc);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);
    auto         userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool         disableVeboxOutput = userFeatureControl->IsVeboxOutputDisabled();
    bool         disableSfc         = userFeatureControl->IsSfcDisabled();
    MOS_FORMAT   midFormat          = Format_A8R8G8B8;

    if (!((SwFilterCgc *)cgc)->IsBt2020ToRGBEnabled())
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_UNIMPLEMENTED);
    }

    FeatureParamCsc *cscParams = &((SwFilterCsc *)csc)->GetSwFilterParams();

    VP_EngineEntry *cscEngine = &((SwFilterCsc *)csc)->GetFilterEngineCaps();

    FeatureParamCgc *cgcParams = &((SwFilterCgc *)cgc)->GetSwFilterParams();

    // Clean usedForNextPass flag.
    if (cscEngine->usedForNextPass)
    {
        cscEngine->usedForNextPass = false;
    }
    if (cscEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("CSC Feature Already been processed, Skip further process");

        PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (m_hwCaps.m_sfcHwEntry[midFormat].cscSupported &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
        m_hwCaps.m_sfcHwEntry[midFormat].inputSupported)
    {
        // Vebox GC + SFC/Render. The csc parameter below if for sfc or render.
        cgcParams->formatOutput     = midFormat;
        cgcParams->dstColorSpace    = CSpace_sRGB;
        cscParams->formatforCUS     = cscParams->formatInput;
        cscParams->formatInput      = cgcParams->formatOutput;
        cscParams->input.colorSpace = cgcParams->dstColorSpace;
        
        cscEngine->bEnabled         = 1;
        if (disableSfc)
        {
            cscEngine->SfcNeeded    = 0;
        }
        else
        {
            cscEngine->SfcNeeded    = 1;
        }
        cscEngine->VeboxNeeded      = 0;
        cscEngine->RenderNeeded     = 1;
        cscEngine->fcSupported      = 1;
    }
    else
    {
        PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
    return MOS_STATUS_SUCCESS;
}

bool Policy::IsDemosaicValidOutputFormat(MOS_FORMAT format)
{
    return (format == Format_R10G10B10A2 || format == Format_A8R8G8B8);
}

bool IsBeCscNeededForAlphaFill(MOS_FORMAT formatInput, MOS_FORMAT formatOutput, PVPHAL_ALPHA_PARAMS compAlpha)
{
    if (nullptr == compAlpha)
    {
        VP_PUBLIC_NORMALMESSAGE("No alpha setting exists.");
        return false;
    }

    if (!IS_ALPHA_FORMAT(formatOutput))
    {
        VP_PUBLIC_NORMALMESSAGE("Alpha setting can be ignored for non-alpha output format.");
        return false;
    }

    if (VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM == compAlpha->AlphaMode)
    {
        VP_PUBLIC_NORMALMESSAGE("No need BeCsc for VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM when output from vebox.");
        return false;
    }

    if (VPHAL_ALPHA_FILL_MODE_OPAQUE == compAlpha->AlphaMode && !IS_ALPHA_FORMAT(formatInput))
    {
        // In such case, vebox will fill the alpha field with 0xff by default.
        VP_PUBLIC_NORMALMESSAGE("No need BeCsc for VPHAL_ALPHA_FILL_MODE_OPAQUE with non-alpha input format.");
        return false;
    }

    return true;
}

bool Policy::IsAlphaSettingSupportedBySfc(MOS_FORMAT formatInput, MOS_FORMAT formatOutput, PVPHAL_ALPHA_PARAMS compAlpha)
{
    if (!IS_ALPHA_FORMAT(formatOutput))
    {
        VP_PUBLIC_NORMALMESSAGE("Alpha setting can be ignored for non-alpha output format.");
        return true;
    }

    if (compAlpha && VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM == compAlpha->AlphaMode)
    {
        if (IS_ALPHA_FORMAT(formatInput))
        {
            VP_PUBLIC_NORMALMESSAGE("VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM is not supported by SFC.");
            if (Format_Y410 == formatOutput)
            {
                // Y410 is also not supported by FC Save_444Scale16_SrcY410 kernel for Alpha Source Mode.
                // Ignore Alpha Source Mode if sfc/render being needed.
                VP_PUBLIC_NORMALMESSAGE("Ignore VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM for Y410.");
                return true;
            }
            return false;
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("VPHAL_ALPHA_FILL_MODE_SOURCE_STREAM can be ignored since no alpha info in input surface.");
            return true;
        }
    }
    else if (compAlpha && VPHAL_ALPHA_FILL_MODE_BACKGROUND == compAlpha->AlphaMode)
    {
        if (IS_ALPHA_FORMAT_RGB8(formatOutput) || IS_ALPHA_FORMAT_RGB10(formatOutput))
        {
            VP_PUBLIC_NORMALMESSAGE("VPHAL_ALPHA_FILL_MODE_BACKGROUND is supported by SFC for RGB8 and RGB10.");
            return true;
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("VPHAL_ALPHA_FILL_MODE_BACKGROUND is not supported for format %d.", formatOutput);
            if (Format_Y410 == formatOutput)
            {
                // Y410 is also not supported by FC Save_444Scale16_SrcY410 kernel for Alpha Background Mode.
                // Ignore Alpha Background Mode if sfc/render being needed.
                VP_PUBLIC_NORMALMESSAGE("Ignore VPHAL_ALPHA_FILL_MODE_BACKGROUND for Y410.");
                return true;
            }
            return false;
        }
    }

    if (compAlpha)
    {
        VP_PUBLIC_NORMALMESSAGE("true for AlphaMode %d.", compAlpha->AlphaMode);
    }
    else
    {
        VP_PUBLIC_NORMALMESSAGE("compAlpha == nullptr. Just return true");
    }

    return true;
}

bool Policy::IsAlphaSettingSupportedByVebox(MOS_FORMAT formatInput, MOS_FORMAT formatOutput, PVPHAL_ALPHA_PARAMS compAlpha)
{
    if (!IS_ALPHA_FORMAT(formatOutput))
    {
        VP_PUBLIC_NORMALMESSAGE("Alpha setting can be ignored for non-alpha output format.");
        return true;
    }

    if (compAlpha && VPHAL_ALPHA_FILL_MODE_BACKGROUND == compAlpha->AlphaMode)
    {
        VP_PUBLIC_NORMALMESSAGE("Alpha Background Mode is not supported by Vebox.");
        return false;
    }
    else
    {
        if (compAlpha)
        {
            VP_PUBLIC_NORMALMESSAGE("true for AlphaMode %d.", compAlpha->AlphaMode);
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("compAlpha == nullptr. Just return true");
        }
        return true;
    }
}

MOS_STATUS Policy::GetCSCExecutionCaps(SwFilter* feature, bool isCamPipeWithBayerInput)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);

    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableVeboxOutput = userFeatureControl->IsVeboxOutputDisabled();
    bool disableSfc         = userFeatureControl->IsSfcDisabled();
    bool veboxTypeH         = userFeatureControl->IsVeboxTypeHMode();
    SwFilterCsc* csc = (SwFilterCsc*)feature;

    FeatureParamCsc *cscParams = &csc->GetSwFilterParams();

    MOS_FORMAT midFormat = Format_Any;

    VP_EngineEntry *cscEngine = &csc->GetFilterEngineCaps();

    cscEngine->isBayerInputInUse = false;
    // Clean usedForNextPass flag.
    if (cscEngine->usedForNextPass)
    {
        cscEngine->usedForNextPass = false;
    }
    if (cscEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("CSC Feature Already been processed, Skip further process");

        PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (cscParams->formatInput == Format_422H ||
        cscParams->formatInput == Format_422V)
    {
        //422H and 422V input not supported by OCL FC yet. Will remove the restriction after they are enabled
        cscEngine->forceLegacyFC = true;
    }

    bool isAlphaSettingSupportedBySfc =
        IsAlphaSettingSupportedBySfc(cscParams->formatInput, cscParams->formatOutput, cscParams->pAlphaParams);
    bool isAlphaSettingSupportedByVebox =
        IsAlphaSettingSupportedByVebox(cscParams->formatInput, cscParams->formatOutput, cscParams->pAlphaParams);

    if (cscParams->formatInput          == cscParams->formatOutput          &&
        cscParams->input.colorSpace     == cscParams->output.colorSpace     &&
        cscParams->input.chromaSiting   == cscParams->output.chromaSiting   &&
        nullptr                         == cscParams->pIEFParams            &&
        isAlphaSettingSupportedByVebox)
    {
        bool veboxSupported = m_hwCaps.m_veboxHwEntry[cscParams->formatInput].inputSupported;
        bool sfcSupported = veboxSupported && m_hwCaps.m_sfcHwEntry[cscParams->formatInput].inputSupported;

        if (!veboxSupported)
        {
            VP_PUBLIC_NORMALMESSAGE("Format %d not supported by vebox. Force to render.", cscParams->formatInput);
            cscEngine->bEnabled     = 1;
            cscEngine->SfcNeeded    = 0;
            cscEngine->VeboxNeeded  = 0;
            cscEngine->RenderNeeded = 1;
            cscEngine->fcSupported  = 1;
            cscEngine->sfcNotSupported = 1;
        }
        else if (disableVeboxOutput)
        {
            VP_PUBLIC_NORMALMESSAGE("Non-csc cases with vebox output disabled. Still keep csc filter to avoid output from vebox.");
            cscEngine->bEnabled             = 1;
            cscEngine->SfcNeeded            = (disableSfc || !sfcSupported) ? 0 : 1;
            cscEngine->VeboxNeeded          = 0;
            cscEngine->RenderNeeded         = 1;
            cscEngine->fcSupported          = 1;
        }
        else if (IsBeCscNeededForAlphaFill(cscParams->formatInput, cscParams->formatOutput, cscParams->pAlphaParams))
        {
            VP_PUBLIC_NORMALMESSAGE("BeCsc is needed by Alpha when output from vebox. Keep csc filter.");
            cscEngine->bEnabled             = 1;
            cscEngine->SfcNeeded            = disableSfc ? 0 : 1; 
            cscEngine->RenderNeeded         = 1;
            cscEngine->fcSupported          = 1;         
            if (veboxTypeH)
            {
                cscEngine->VeboxNeeded = 0;
                VP_PUBLIC_NORMALMESSAGE("Not use vebox to do csc for veboxTypeH.");
            }
            else
            {
                cscEngine->VeboxNeeded = 1;
            }
        }
        else
        {
            // for non-csc cases, all engine supported
            cscEngine->bEnabled             = 0;
            cscEngine->SfcNeeded            = 0;
            cscEngine->VeboxNeeded          = 0;
            cscEngine->RenderNeeded         = 0;
            if (sfcSupported && !disableSfc)
            {
                cscEngine->forceEnableForSfc = 1;
            }
            else
            {
                cscEngine->sfcNotSupported = 1;
            }
        }

        PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (IS_COLOR_SPACE_BT2020_YUV(cscParams->input.colorSpace))
    {
        if ((cscParams->output.colorSpace == CSpace_BT601)              ||
            (cscParams->output.colorSpace == CSpace_BT709)              ||
            (cscParams->output.colorSpace == CSpace_BT601_FullRange)    ||
            (cscParams->output.colorSpace == CSpace_BT709_FullRange)    ||
            (cscParams->output.colorSpace == CSpace_stRGB)              ||
            (cscParams->output.colorSpace == CSpace_sRGB))
        {
            // Should not come here for BT2020 support.
            VP_PUBLIC_ASSERTMESSAGE("not support BT2020 input.");
            return MOS_STATUS_UNIMPLEMENTED;
        }
    }

    cscEngine->bEnabled     = 1;
    cscEngine->RenderNeeded = 1;
    cscEngine->fcSupported  = 1;

    // SFC CSC enabling check
    if (!disableSfc                                                    &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatInput].inputSupported   &&
        (m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported & 
            VpGetFormatTileSupport(cscParams->output.tileMode))        &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatInput].cscSupported     &&
        isAlphaSettingSupportedBySfc)
    {
        cscEngine->SfcNeeded = 1;
    }

    if (disableVeboxOutput)
    {
        // If vebox output disabled, then such mega feature like CSC will not take effect in Vebox.
        // then CSC will must be assign to SFC/Render path for execution.
        VP_PUBLIC_NORMALMESSAGE("Force to use sfc or render for csc.");
    }
    else
    {
        if (!cscParams->pIEFParams                                                            &&
            m_hwCaps.m_veboxHwEntry[cscParams->formatInput].inputSupported                    &&
            (m_hwCaps.m_veboxHwEntry[cscParams->formatOutput].outputSupported ||
            (isCamPipeWithBayerInput && IsDemosaicValidOutputFormat(cscParams->formatOutput))) &&
            m_hwCaps.m_veboxHwEntry[cscParams->formatInput].iecp                              &&
            m_hwCaps.m_veboxHwEntry[cscParams->formatInput].backEndCscSupported               &&
            isAlphaSettingSupportedByVebox)
        {
            cscEngine->VeboxNeeded = 1;
        }
    }

    PrintFeatureExecutionCaps(__FUNCTION__, *cscEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetScalingExecutionCapsHdr(SwFilter *feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(GetScalingExecutionCaps(feature, true, false));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetScalingExecutionCaps(SwFilter *feature, bool isDIEnabled)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_STATUS_RETURN(GetScalingExecutionCaps(feature, false, isDIEnabled));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetScalingExecutionCaps(SwFilter *feature, bool isHdrEnabled, bool isDIEnabled)
{
    VP_FUNC_CALL();

    uint32_t dwSurfaceWidth = 0, dwSurfaceHeight = 0;
    uint32_t dwOutputSurfaceWidth = 0, dwOutputSurfaceHeight = 0;
    uint32_t veboxMinWidth = 0, veboxMaxWidth = 0;
    uint32_t veboxMinHeight = 0, veboxMaxHeight = 0;
    uint32_t dwSfcInputMinWidth = 0, dwSfcMaxWidth = 0;
    uint32_t dwSfcInputMinHeight = 0, dwSfcMaxHeight = 0;
    uint32_t dwSfcOutputMinWidth = 0, dwSfcOutputMinHeight = 0;
    uint32_t dwDstMinHeight = 0;
    float    fScaleMin = 0, fScaleMax = 0;
    float    fScaleMin2Pass = 0, fScaleMax2Pass = 0;
    float    fScaleX = 0, fScaleY = 0;

    VP_PUBLIC_CHK_NULL_RETURN(feature);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);

    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableSfc = userFeatureControl->IsSfcDisabled();
    SwFilterScaling* scaling = (SwFilterScaling*)feature;
    FeatureParamScaling *scalingParams = &scaling->GetSwFilterParams();
    VP_EngineEntry *scalingEngine      = &scaling->GetFilterEngineCaps();
    bool isAlphaSettingSupportedBySfc =
        IsAlphaSettingSupportedBySfc(scalingParams->formatInput, scalingParams->formatOutput, scalingParams->pCompAlpha);
    bool isAlphaSettingSupportedByVebox =
        IsAlphaSettingSupportedByVebox(scalingParams->formatInput, scalingParams->formatOutput, scalingParams->pCompAlpha);

    if (scalingParams->formatOutput == Format_422H)
    {
        VP_PUBLIC_ASSERTMESSAGE("Scaling not support 422H format output.");
        return MOS_STATUS_UNIMPLEMENTED;
    }

    // Clean usedForNextPass flag.
    if (scalingEngine->usedForNextPass)
    {
        scalingEngine->usedForNextPass = false;
    }

    if (scalingEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");

        PrintFeatureExecutionCaps(__FUNCTION__, *scalingEngine);
        return MOS_STATUS_SUCCESS;
    }

    // For AVS sampler not enabled case, HQ/Fast scaling should go to SFC.
    // And Ief should only be done by SFC.
    if (!m_hwCaps.m_rules.isAvsSamplerSupported &&
        scalingParams->scalingPreference != VPHAL_SCALING_PREFER_SFC)
    {
        VP_PUBLIC_NORMALMESSAGE("Force scalingPreference from %d to SFC", scalingParams->scalingPreference);
        scalingParams->scalingPreference = VPHAL_SCALING_PREFER_SFC;
    }

    if (userFeatureControl->IsSFCLinearOutputByTileConvertEnabled() &&
        scalingParams->output.dwWidth <= 1152        &&
        scalingParams->output.dwPitch != 128         &&
        scalingParams->output.dwPitch != 256         &&
        (scalingParams->output.dwPitch % 1024) !=0   &&
        !(scalingParams->formatOutput == Format_RGBP ||
         scalingParams->formatOutput == Format_BGRP  ||
         scalingParams->formatOutput == Format_R8G8B8) &&
        scalingParams->output.tileMode == MOS_TILE_LINEAR_GMM)
    {
        scalingEngine->enableSFCLinearOutputByTileConvert = true;
        VP_PUBLIC_NORMALMESSAGE("enable sfcLinearOutputByTileConvert, output width %d, output pitch %d, output tilemode %d", scalingParams->output.dwWidth, scalingParams->output.dwPitch, scalingParams->output.tileMode);
    }
    dwSurfaceWidth        = scalingParams->input.dwWidth;
    dwSurfaceHeight       = scalingParams->input.dwHeight;
    dwOutputSurfaceWidth  = scalingParams->output.dwWidth;
    dwOutputSurfaceHeight = scalingParams->output.dwHeight;

    // Region of the input frame which needs to be processed by SFC
    uint32_t dwSourceRegionHeight = MOS_ALIGN_FLOOR(
        MOS_MIN((uint32_t)(scalingParams->input.rcSrc.bottom - scalingParams->input.rcSrc.top), dwSurfaceHeight),
        m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].verticalAlignUnit);
    uint32_t dwSourceRegionWidth = MOS_ALIGN_FLOOR(
        MOS_MIN((uint32_t)(scalingParams->input.rcSrc.right - scalingParams->input.rcSrc.left), dwSurfaceWidth),
        m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].horizontalAlignUnit);

    // Size of the Output Region over the Render Target
    uint32_t dwOutputRegionHeight = MOS_ALIGN_CEIL(
        (uint32_t)(scalingParams->input.rcDst.bottom - scalingParams->input.rcDst.top),
        m_hwCaps.m_sfcHwEntry[scalingParams->formatOutput].verticalAlignUnit);
    uint32_t dwOutputRegionWidth = MOS_ALIGN_CEIL(
        (uint32_t)(scalingParams->input.rcDst.right - scalingParams->input.rcDst.left),
        m_hwCaps.m_sfcHwEntry[scalingParams->formatOutput].horizontalAlignUnit);

    if (!m_hwCaps.m_veboxHwEntry[scalingParams->formatInput].inputSupported)
    {
        // For non-scaling cases with vebox unsupported format, will force to use fc.
        scalingEngine->bEnabled          = (dwOutputRegionHeight != dwSourceRegionHeight ||
                                            dwOutputRegionWidth != dwSourceRegionWidth);
        scalingEngine->SfcNeeded         = 0;
        scalingEngine->VeboxNeeded       = 0;
        scalingEngine->RenderNeeded      = 1;
        scalingEngine->fcSupported       = 1;
        scalingEngine->hdrKernelSupported = 1;
        scalingEngine->forceEnableForSfc = 0;
        scalingEngine->forceEnableForFc  = 1;
        scalingEngine->veboxNotSupported = 1;

        PrintFeatureExecutionCaps(__FUNCTION__, *scalingEngine);
        return MOS_STATUS_SUCCESS;
    }

    veboxMinWidth        = m_hwCaps.m_veboxHwEntry[scalingParams->formatInput].minWidth;
    veboxMaxWidth        = m_hwCaps.m_veboxHwEntry[scalingParams->formatInput].maxWidth;
    veboxMinHeight       = m_hwCaps.m_veboxHwEntry[scalingParams->formatInput].minHeight;
    veboxMaxHeight       = m_hwCaps.m_veboxHwEntry[scalingParams->formatInput].maxHeight;
    dwSfcInputMinWidth   = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].inputMinResolution;
    dwSfcMaxWidth        = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].maxResolution;
    dwSfcInputMinHeight  = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].inputMinResolution;
    dwSfcMaxHeight       = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].maxResolution;
    dwSfcOutputMinWidth  = m_hwCaps.m_sfcHwEntry[scalingParams->formatOutput].outputMinResolution;
    dwSfcOutputMinHeight = m_hwCaps.m_sfcHwEntry[scalingParams->formatOutput].outputMinResolution;
    fScaleMin            = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].minScalingRatio;
    fScaleMax            = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].maxScalingRatio;
    if (m_hwCaps.m_rules.sfcMultiPassSupport.scaling.enable)
    {
        fScaleMin2Pass = fScaleMin * m_hwCaps.m_rules.sfcMultiPassSupport.scaling.downScaling.minRatioEnlarged;
        fScaleMax2Pass = fScaleMax * m_hwCaps.m_rules.sfcMultiPassSupport.scaling.upScaling.maxRatioEnlarged;
    }

    if (scalingParams->interlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED)
    {
        dwDstMinHeight = dwSfcOutputMinHeight * 2;
    }
    else
    {
        dwDstMinHeight = dwSfcOutputMinHeight;
    }

    if (OUT_OF_BOUNDS(dwSurfaceWidth, veboxMinWidth, veboxMaxWidth) ||
        OUT_OF_BOUNDS(dwSurfaceHeight, veboxMinHeight, veboxMaxHeight))
    {
        // For non-scaling cases with vebox unsupported format, will force to use fc.
        scalingEngine->bEnabled          = (dwOutputRegionHeight != dwSourceRegionHeight ||
                                            dwOutputRegionWidth != dwSourceRegionWidth);
        scalingEngine->SfcNeeded         = 0;
        scalingEngine->VeboxNeeded       = 0;
        scalingEngine->forceEnableForSfc = 0;
        scalingEngine->RenderNeeded      = 1;
        scalingEngine->fcSupported       = 1;
        scalingEngine->hdrKernelSupported = 1;
        scalingEngine->forceEnableForFc  = 1;
        scalingEngine->veboxNotSupported = 1;

        VP_PUBLIC_NORMALMESSAGE("The surface resolution (%d x %d) is not supported by vebox (%d x %d) ~ (%d x %d).",
            dwSurfaceWidth, dwSurfaceHeight, veboxMinWidth, veboxMinHeight, veboxMaxWidth, veboxMaxHeight);

        PrintFeatureExecutionCaps(__FUNCTION__, *scalingEngine);
        return MOS_STATUS_SUCCESS;
    }

    // Calculate the scaling ratio
    // Both source region and scaled region are pre-rotated
    // Need to take Interlace scaling into consideration next step
    fScaleX = (float)dwOutputRegionWidth / (float)dwSourceRegionWidth;
    fScaleY = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;

    auto isSfcIscalingNotSupported = [&]()
    {
        return ISCALING_INTERLEAVED_TO_INTERLEAVED == scalingParams->interlacedScalingType &&
               (scalingParams->input.rcSrc.left    != 0 ||
                   scalingParams->input.rcSrc.top  != 0 ||
                   scalingParams->input.rcDst.left != 0 ||
                   scalingParams->input.rcDst.top  != 0 ||
                   scalingParams->rotation.rotationNeeded);
    };

    if (fScaleX == 1.0f && fScaleY == 1.0f &&
        // Only support vebox crop from left-top, which is to align with legacy path.
        0 == scalingParams->input.rcSrc.left && 0 == scalingParams->input.rcSrc.top &&
        SAME_SIZE_RECT(scalingParams->input.rcDst, scalingParams->output.rcDst) &&
        // If alpha is not supported by vebox, which should go SFC pipe.
        isAlphaSettingSupportedByVebox &&
        // If Colorfill enabled, which should go SFC pipe.
        !IsColorfillEnabled(scalingParams) &&
        scalingParams->interlacedScalingType != ISCALING_INTERLEAVED_TO_FIELD &&
        scalingParams->interlacedScalingType != ISCALING_FIELD_TO_INTERLEAVED)
    {
        if (OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcInputMinWidth, dwSfcMaxWidth)    ||
                OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcInputMinHeight, dwSfcMaxHeight))
        {
            // for non-Scaling cases, all engine supported
            scalingEngine->bEnabled             = 0;
            scalingEngine->SfcNeeded            = 0;
            scalingEngine->VeboxNeeded          = 0;
            scalingEngine->RenderNeeded         = 0;
            scalingEngine->forceEnableForSfc    = 0;
            scalingEngine->forceEnableForFc     = 1;
            scalingEngine->fcSupported          = 1;
            scalingEngine->hdrKernelSupported   = 1;
            scalingEngine->sfcNotSupported      = 1;
            VP_PUBLIC_NORMALMESSAGE("The surface resolution (%d x %d) is not supported by sfc (%d x %d) ~ (%d x %d).",
                dwSurfaceWidth, dwSurfaceHeight, dwSfcInputMinWidth, dwSfcInputMinHeight, dwSfcMaxWidth, dwSfcMaxHeight);
        }
        else if (isSfcIscalingNotSupported())
        {
            scalingEngine->bEnabled        = 1;
            scalingEngine->SfcNeeded       = 0;
            scalingEngine->VeboxNeeded     = 0;
            scalingEngine->RenderNeeded    = 1;
            scalingEngine->fcSupported     = 1;
            scalingEngine->sfcNotSupported = 1;
            VP_PUBLIC_NORMALMESSAGE("Interlaced scaling cannot support offset, rotate or mirror by SFC pipe, fallback to FC.");
        }
        else if (scalingParams->input.rcDst.left > 0    ||
                 scalingParams->input.rcDst.top  > 0)
        {
            // for dst left and top non zero cases, should go SFC or Render Pipe
            scalingEngine->bEnabled             = 1;
            scalingEngine->SfcNeeded            = !disableSfc && isAlphaSettingSupportedBySfc;
            scalingEngine->VeboxNeeded          = 0;
            scalingEngine->RenderNeeded         = 1;
            scalingEngine->fcSupported          = 1;
            scalingEngine->hdrKernelSupported   = 1;
            VP_PUBLIC_NORMALMESSAGE("The dst left and top non zero is not supported by vebox ");
        }
        else
        {
            // for non-Scaling cases, all engine supported
            scalingEngine->bEnabled             = 0;
            scalingEngine->SfcNeeded            = 0;
            scalingEngine->VeboxNeeded          = 0;
            scalingEngine->RenderNeeded         = 0;
            scalingEngine->forceEnableForSfc    = isAlphaSettingSupportedBySfc;
            scalingEngine->forceEnableForFc     = 1;
            scalingEngine->fcSupported          = 1;
            scalingEngine->hdrKernelSupported   = 1;
            scalingEngine->sfcNotSupported      = disableSfc || !isAlphaSettingSupportedBySfc;
        }

        PrintFeatureExecutionCaps(__FUNCTION__, *scalingEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (disableSfc)
    {
        scalingEngine->bEnabled     = 1;
        scalingEngine->RenderNeeded = 1;
        scalingEngine->fcSupported  = 1;
        scalingEngine->hdrKernelSupported = 1;
        scalingEngine->SfcNeeded    = 0;
        // Set sfcNotSupported to 1 to avoid SFC being selected without scaling filter.
        scalingEngine->sfcNotSupported = 1;
        VP_PUBLIC_NORMALMESSAGE("Force scaling to FC. disableSfc %d", disableSfc);
        PrintFeatureExecutionCaps(__FUNCTION__, *scalingEngine);
        return MOS_STATUS_SUCCESS;
    }

    // SFC Scaling enabling check
    if (m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].inputSupported   &&
        (m_hwCaps.m_sfcHwEntry[scalingParams->formatOutput].outputSupported & VpGetFormatTileSupport(scalingParams->output.tileMode) ) &&
        m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].scalingSupported)
    {
        if (!(OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcInputMinWidth, dwSfcMaxWidth)         ||
              OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcInputMinHeight, dwSfcMaxHeight)      ||
              OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcInputMinWidth, dwSfcMaxWidth)    ||
              OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcInputMinHeight, dwSfcMaxHeight) ||
              OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcOutputMinWidth, dwSfcMaxWidth)    ||
              OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcOutputMinHeight, dwSfcMaxHeight) ||
              OUT_OF_BOUNDS(dwOutputSurfaceWidth, dwSfcOutputMinWidth, dwSfcMaxWidth)   ||
              OUT_OF_BOUNDS(dwOutputSurfaceHeight, dwDstMinHeight, dwSfcMaxHeight)))
        {
            if ((m_hwCaps.m_rules.sfcMultiPassSupport.scaling.enable             &&
                (OUT_OF_BOUNDS(fScaleX, fScaleMin2Pass, fScaleMax2Pass)          ||
                 OUT_OF_BOUNDS(fScaleY, fScaleMin2Pass, fScaleMax2Pass)))        ||
                ((!m_hwCaps.m_rules.sfcMultiPassSupport.scaling.enable           ||
                  isHdrEnabled)                                                  &&   // Ve HDR + (8, 16] && [1/16, 1/8) scaling on render, not use 2 Pass SFC for scaling,
                (OUT_OF_BOUNDS(fScaleX, fScaleMin, fScaleMax)                    ||   // since A10R10G10B10/ARGB8 cannot be used as the input of vebox when global IECP being enabled.
                 OUT_OF_BOUNDS(fScaleY, fScaleMin, fScaleMax)))                  ||
                (scalingParams->scalingPreference == VPHAL_SCALING_PREFER_COMP))
            {
                // Render Pipe, need to add more conditions next step for multiple SFC mode
                // if Render didn't have AVS but Scaling quality mode needed
                scalingEngine->bEnabled     = 1;
                scalingEngine->RenderNeeded = 1;
                scalingEngine->fcSupported  = 1;
                scalingEngine->hdrKernelSupported = 1;
                scalingEngine->SfcNeeded    = 0;
                // Set sfcNotSupported to 1 to avoid SFC being selected without scaling filter.
                scalingEngine->sfcNotSupported = 1;
                VP_PUBLIC_NORMALMESSAGE("Fc selected. fScaleX %f, fScaleY %f, scalingPreference %d",
                    fScaleX, fScaleY, scalingParams->scalingPreference);
            }
            else if (isSfcIscalingNotSupported())
            {
                scalingEngine->bEnabled        = 1;
                scalingEngine->RenderNeeded    = 1;
                scalingEngine->fcSupported     = 1;
                scalingEngine->SfcNeeded       = 0;
                scalingEngine->sfcNotSupported = 1;
                VP_PUBLIC_NORMALMESSAGE("Interlaced scaling cannot support offset, rotate or mirror by SFC pipe, fallback to FC.");
            }
            // SFC feasible
            else
            {
                bool sfc2PassScalingNeededX = OUT_OF_BOUNDS(fScaleX, fScaleMin, fScaleMax);
                bool sfc2PassScalingNeededY = OUT_OF_BOUNDS(fScaleY, fScaleMin, fScaleMax);
                bool multiPassNeeded= sfc2PassScalingNeededX || sfc2PassScalingNeededY;

                scalingEngine->bEnabled = 1;

                if (multiPassNeeded && isDIEnabled)
                {
                    scalingEngine->RenderNeeded = 1;
                    scalingEngine->fcSupported  = 1;
                    // Set sfcNotSupported to 1 to avoid SFC being selected without scaling filter.
                    scalingEngine->sfcNotSupported = 1;
                    VP_PUBLIC_NORMALMESSAGE("2 pass scaling + DI switch to render path.");
                }
                else if (!m_hwCaps.m_rules.isAvsSamplerSupported && scalingParams->isPrimary && isAlphaSettingSupportedBySfc)
                {
                    // For primary layer, force to use sfc for better quailty.
                    scalingEngine->SfcNeeded = 1;
                    scalingEngine->sfc2PassScalingNeededX = sfc2PassScalingNeededX ? 1 : 0;
                    scalingEngine->sfc2PassScalingNeededY = sfc2PassScalingNeededY ? 1 : 0;
                    scalingEngine->multiPassNeeded        = multiPassNeeded;

                    if (1 == fScaleX && 1 == fScaleY)
                    {
                        VP_PUBLIC_NORMALMESSAGE("Fc can be selected for scale ratio being 1 case on primary, e.g. crop only case.");
                        scalingEngine->RenderNeeded = 1;
                        scalingEngine->fcSupported  = 1;
                        scalingEngine->hdrKernelSupported = 1;
                    }
                    else
                    {
                        scalingEngine->hdrKernelSupported = 1;
                    }
                }
                else
                {
                    scalingEngine->RenderNeeded = 1;
                    scalingEngine->fcSupported  = 1;
                    scalingEngine->hdrKernelSupported = 1;
                    // For non-primary layer, only consider sfc for non-2-pass case.
                    if (!sfc2PassScalingNeededX && !sfc2PassScalingNeededY)
                    {
                        scalingEngine->SfcNeeded = isAlphaSettingSupportedBySfc;
                    }
                    else
                    {
                        // Set sfcNotSupported to 1 to avoid SFC being selected without scaling filter.
                        scalingEngine->sfcNotSupported = 1;
                    }
                }
            }
        }
        else
        {
            scalingEngine->bEnabled     = 1;
            scalingEngine->RenderNeeded = 1;
            scalingEngine->fcSupported  = 1;
            scalingEngine->hdrKernelSupported = 1;
            scalingEngine->SfcNeeded    = 0;
            // Set sfcNotSupported to 1 to avoid SFC being selected without scaling filter.
            scalingEngine->sfcNotSupported = 1;
            VP_PUBLIC_NORMALMESSAGE("Scaling parameters are not supported by SFC. Switch to Render.");
        }
    }
    else
    {
        scalingEngine->bEnabled     = 1;
        scalingEngine->RenderNeeded = 1;
        scalingEngine->fcSupported  = 1;
        scalingEngine->hdrKernelSupported = 1;
        scalingEngine->SfcNeeded    = 0;
        VP_PUBLIC_NORMALMESSAGE("Format is not supported by SFC. Switch to Render.");
    }

    PrintFeatureExecutionCaps(__FUNCTION__, *scalingEngine);
    return MOS_STATUS_SUCCESS;
}

bool Policy::IsSfcRotationSupported(FeatureParamRotMir *rotationParams)
{
    VP_FUNC_CALL();

    bool isSfcRotationSupported = false;
    if (m_hwCaps.m_sfcHwEntry[rotationParams->formatInput].inputSupported &&
        m_hwCaps.m_sfcHwEntry[rotationParams->formatOutput].outputSupported)
    {
        if (VPHAL_ROTATION_IDENTITY == rotationParams->rotation)
        {
            isSfcRotationSupported = true;
        }
        else if (VPHAL_MIRROR_HORIZONTAL == rotationParams->rotation)
        {
            if (m_hwCaps.m_sfcHwEntry[rotationParams->formatInput].mirrorSupported)
            {
                isSfcRotationSupported = true;
            }
        }
        else if (rotationParams->rotation <= VPHAL_ROTATION_270)
        {
            // Rotation w/o mirror case
            if (m_hwCaps.m_sfcHwEntry[rotationParams->formatInput].rotationSupported &&
                rotationParams->surfInfo.tileOutput == MOS_TILE_Y)
            {
                isSfcRotationSupported = true;
            }
        }
        else
        {
            // Rotation w/ mirror case
            if (m_hwCaps.m_sfcHwEntry[rotationParams->formatInput].mirrorSupported &&
                m_hwCaps.m_sfcHwEntry[rotationParams->formatInput].rotationSupported &&
                rotationParams->surfInfo.tileOutput == MOS_TILE_Y)
            {
                isSfcRotationSupported = true;
            }
        }
    }

    VP_PUBLIC_NORMALMESSAGE("Is rotation supported by sfc: %d", isSfcRotationSupported ? 1 : 0);
    return isSfcRotationSupported;
}

MOS_STATUS Policy::GetRotationExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(feature);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);

    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableSfc = userFeatureControl->IsSfcDisabled();
    SwFilterRotMir* rotation = (SwFilterRotMir*)feature;
    FeatureParamRotMir *rotationParams = &rotation->GetSwFilterParams();
    VP_EngineEntry *rotationEngine = &rotation->GetFilterEngineCaps();

    // Clean usedForNextPass flag.
    if (rotationEngine->usedForNextPass)
    {
        rotationEngine->usedForNextPass = false;
    }
    if (rotationEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");

        PrintFeatureExecutionCaps(__FUNCTION__, *rotationEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (rotationParams->rotation == VPHAL_ROTATION_IDENTITY)
    {
        // for non-rotation cases, all engine supported
        rotationEngine->bEnabled                = 0;
        rotationEngine->VeboxNeeded             = 0;
        rotationEngine->SfcNeeded               = 0;
        rotationEngine->RenderNeeded            = 0;
        rotationEngine->forceEnableForSfc       = 1;
        rotationEngine->forceEnableForHdrKernel = 1;
        PrintFeatureExecutionCaps(__FUNCTION__, *rotationEngine);
        return MOS_STATUS_SUCCESS;
    }

    rotationEngine->bEnabled        = 1;
    rotationEngine->RenderNeeded    = 1;
    rotationEngine->fcSupported     = 1;
    rotationEngine->hdrKernelSupported = 1;

    if (disableSfc)
    {
        VP_PUBLIC_NORMALMESSAGE("Force rotation to FC. disableSfc %d", disableSfc);
        PrintFeatureExecutionCaps(__FUNCTION__, *rotationEngine);
        return MOS_STATUS_SUCCESS;
    }

    rotationEngine->SfcNeeded       = IsSfcRotationSupported(rotationParams);

    PrintFeatureExecutionCaps(__FUNCTION__, *rotationEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetDenoiseExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterDenoise* denoise = dynamic_cast<SwFilterDenoise*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(denoise);

    FeatureParamDenoise& denoiseParams = denoise->GetSwFilterParams();
    VP_EngineEntry& denoiseEngine = denoise->GetFilterEngineCaps();
    MOS_FORMAT inputformat = denoiseParams.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    uint32_t        widthAlignUint  = m_hwCaps.m_veboxHwEntry[inputformat].horizontalAlignUnit;
    uint32_t        heightAlignUnit = m_hwCaps.m_veboxHwEntry[inputformat].verticalAlignUnit;

    if (denoiseEngine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, denoiseEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (m_hwCaps.m_veboxHwEntry[inputformat].denoiseSupported)
    {
        if (denoiseParams.denoiseParams.bEnableHVSDenoise)
        {
            denoiseParams.stage      = DN_STAGE_HVS_KERNEL;
            denoiseEngine.bEnabled   = 1;
            denoiseEngine.isolated   = 1;
            denoiseEngine.RenderNeeded = 1;
            denoise->SetRenderTargetType(RenderTargetType::RenderTargetTypeParameter);
            VP_PUBLIC_NORMALMESSAGE("DN_STAGE_HVS_KERNEL");
        }
        else
        {
            denoiseParams.stage = DN_STAGE_DEFAULT;
            widthAlignUint = MOS_ALIGN_CEIL(m_hwCaps.m_veboxHwEntry[inputformat].horizontalAlignUnit, 2);

            if (inputformat == Format_NV12 ||
                inputformat == Format_P010 ||
                inputformat == Format_P016)
            {
                heightAlignUnit = MOS_ALIGN_CEIL(m_hwCaps.m_veboxHwEntry[inputformat].verticalAlignUnit, 4);
            }
            else
            {
                heightAlignUnit = MOS_ALIGN_CEIL(m_hwCaps.m_veboxHwEntry[inputformat].verticalAlignUnit, 2);
            }

            if (MOS_IS_ALIGNED(MOS_MIN(denoiseParams.heightInput, denoiseParams.srcBottom), heightAlignUnit))
            {
                denoiseEngine.bEnabled    = 1;
                denoiseEngine.VeboxNeeded = 1;
            }
            else
            {
                VP_PUBLIC_NORMALMESSAGE("Denoise Feature is disabled since min of heightInput (%d) and srcBottom (%d) not being %d aligned.", denoiseParams.heightInput, denoiseParams.srcBottom, heightAlignUnit);
            }
        }
    }

    denoiseParams.widthAlignUnitInput = widthAlignUint;
    denoiseParams.heightAlignUnitInput = heightAlignUnit;

    PrintFeatureExecutionCaps(__FUNCTION__, denoiseEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetDeinterlaceExecutionCaps(SwFilter* feature, bool forceDIToRender)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterDeinterlace *swFilterDi = dynamic_cast<SwFilterDeinterlace*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(swFilterDi);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);
    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableVeboxOutput = userFeatureControl->IsVeboxOutputDisabled();
    bool disableSfc        = userFeatureControl->IsSfcDisabled();

    FeatureParamDeinterlace &diParams = swFilterDi->GetSwFilterParams();
    VP_EngineEntry &diEngine = swFilterDi->GetFilterEngineCaps();
    MOS_FORMAT      inputformat = diParams.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (diEngine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("DI Feature Already been processed, Skip further process.");
        PrintFeatureExecutionCaps(__FUNCTION__, diEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (!m_hwCaps.m_veboxHwEntry[inputformat].deinterlaceSupported)
    {
        diEngine.bEnabled     = 1;
        diEngine.RenderNeeded = 1;
        diEngine.fcSupported  = 1;
        diEngine.VeboxNeeded  = 0;
        PrintFeatureExecutionCaps(__FUNCTION__, diEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (nullptr == diParams.diParams                                            ||                                                    
        (!MOS_IS_ALIGNED(MOS_MIN((uint32_t)diParams.heightInput, (uint32_t)diParams.rcSrc.bottom), 4) &&
        (diParams.formatInput == Format_P010                                    ||
         diParams.formatInput == Format_P016                                    ||
         diParams.formatInput == Format_NV12)))
    {
        diEngine.bEnabled     = 0;
        diEngine.RenderNeeded = 0;
        diEngine.fcSupported  = 0;
        diEngine.VeboxNeeded  = 0;
        PrintFeatureExecutionCaps(__FUNCTION__, diEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (forceDIToRender && diParams.diParams)
    {
        diEngine.bEnabled             = 1;
        diEngine.RenderNeeded         = 1;
        diEngine.fcSupported          = 1;
        diEngine.VeboxNeeded          = 0;
        VP_PUBLIC_NORMALMESSAGE("force to render for 2 pass scaling + DI.");
        PrintFeatureExecutionCaps(__FUNCTION__, diEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (m_vpInterface.GetResourceManager()->IsRefValid()    &&
        m_vpInterface.GetResourceManager()->IsSameSamples())
    {
        diEngine.bypassVeboxFeatures    = 1;
        diEngine.diProcess2ndField      = 1;
    }
    else
    {
        if (diParams.diParams->DIMode == DI_MODE_BOB)
        {
            diEngine.bEnabled     = 1;
            diEngine.RenderNeeded = 1;
            diEngine.fcSupported  = 1;

            // to align with case design, if disableSfc == 1, force di to render
            if (disableSfc)
            {
                diEngine.VeboxNeeded = 0;
                VP_PUBLIC_NORMALMESSAGE("Force DI to render.");
            }
            else
            {
                diEngine.VeboxNeeded = 1;
            }
        }
        else
        {
            diEngine.bEnabled     = 1;
            diEngine.RenderNeeded = 0;
            diEngine.fcSupported  = 0;
            diEngine.VeboxNeeded  = 1;
        }
    }

    PrintFeatureExecutionCaps(__FUNCTION__, diEngine);
    VP_PUBLIC_NORMALMESSAGE("Di mode = %d", diParams.diParams->DIMode);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetSteExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterSte* steFilter = dynamic_cast<SwFilterSte*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(steFilter);

    FeatureParamSte& steParams = steFilter->GetSwFilterParams();
    VP_EngineEntry& steEngine = steFilter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = steParams.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (steEngine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("ACE Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, steEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (m_hwCaps.m_veboxHwEntry[inputformat].steSupported   &&
        m_hwCaps.m_veboxHwEntry[inputformat].inputSupported &&
        m_hwCaps.m_veboxHwEntry[inputformat].iecp)
    {
        steEngine.bEnabled = 1;
        steEngine.VeboxNeeded = 1;
        steEngine.VeboxIECPNeeded = 1;
    }

    PrintFeatureExecutionCaps(__FUNCTION__, steEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetTccExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterTcc* tccFilter = dynamic_cast<SwFilterTcc*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(tccFilter);

    FeatureParamTcc& tccParams = tccFilter->GetSwFilterParams();
    VP_EngineEntry& tccEngine = tccFilter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = tccParams.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (tccEngine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("TCC Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, tccEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (m_hwCaps.m_veboxHwEntry[inputformat].inputSupported &&
        m_hwCaps.m_veboxHwEntry[inputformat].iecp           &&
        m_hwCaps.m_veboxHwEntry[inputformat].tccSupported)
    {
        tccEngine.bEnabled = 1;
        tccEngine.VeboxNeeded = 1;
        tccEngine.VeboxIECPNeeded = 1;
    }

    PrintFeatureExecutionCaps(__FUNCTION__, tccEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetProcampExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterProcamp* procampFilter = dynamic_cast<SwFilterProcamp*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(procampFilter);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);

    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    FeatureParamProcamp& procampParams = procampFilter->GetSwFilterParams();
    VP_EngineEntry& procampEngine = procampFilter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = procampParams.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (procampEngine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Procamp Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, procampEngine);
        return MOS_STATUS_SUCCESS;
    }

    procampEngine.bEnabled = 1;
    procampEngine.RenderNeeded = 1;
    procampEngine.fcSupported = 1;

    if (m_hwCaps.m_veboxHwEntry[inputformat].inputSupported &&
        m_hwCaps.m_veboxHwEntry[inputformat].iecp)
    {
        procampEngine.VeboxNeeded = 1;
        procampEngine.VeboxIECPNeeded = 1;
    }

    PrintFeatureExecutionCaps(__FUNCTION__, procampEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetHdrExecutionCaps(SwFilter *feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);

    SwFilterHdr *hdrFilter = dynamic_cast<SwFilterHdr *>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(hdrFilter);

    FeatureParamHdr *hdrParams = &hdrFilter->GetSwFilterParams();

    VP_EngineEntry *pHDREngine  = &hdrFilter->GetFilterEngineCaps();
    MOS_FORMAT      inputformat = hdrParams->formatInput;
    uint32_t dwSurfaceWidth = 0, dwSurfaceHeight = 0;
    uint32_t veboxMinWidth = 0, veboxMaxWidth = 0;
    uint32_t veboxMinHeight = 0, veboxMaxHeight = 0;
    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableVeboxOutput = userFeatureControl->IsVeboxOutputDisabled();
    bool disableSfc = userFeatureControl->IsSfcDisabled();

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (pHDREngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("HDR Feature Already been processed, Skip further process");
        if (HDR_STAGE_VEBOX_3DLUT_UPDATE == hdrParams->stage)
        {
            VP_PUBLIC_NORMALMESSAGE("HDR_STAGE_VEBOX_3DLUT_UPDATE");
        }
        PrintFeatureExecutionCaps(__FUNCTION__, *pHDREngine);
        return MOS_STATUS_SUCCESS;
    }

    veboxMinWidth  = m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].minWidth;
    veboxMaxWidth  = m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].maxWidth;
    veboxMinHeight = m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].minHeight;
    veboxMaxHeight = m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].maxHeight;


    if (disableVeboxOutput && disableSfc                                        ||
        OUT_OF_BOUNDS(hdrParams->widthInput, veboxMinWidth, veboxMaxWidth)      ||
        OUT_OF_BOUNDS(hdrParams->heightInput, veboxMinHeight, veboxMaxHeight)   ||
        !m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].inputSupported         ||
        !m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].hdrSupported)
    {
        pHDREngine->bEnabled            = 1;
        pHDREngine->VeboxNeeded         = 0;
        pHDREngine->RenderNeeded        = 1;
        pHDREngine->hdrKernelNeeded     = 1;
        pHDREngine->hdrKernelSupported  = 1;
        VP_PUBLIC_NORMALMESSAGE("Hdr render is selected. Input Resolution %dx%d, Input Format %d",
            hdrParams->widthInput, hdrParams->heightInput, hdrParams->formatInput);
        PrintFeatureExecutionCaps(__FUNCTION__, *pHDREngine);
        return MOS_STATUS_SUCCESS;
    }

    if (IsHDR33LutSizeSupported() && (hdrParams->hdrMode == VPHAL_HDR_MODE_H2H))
    {
        hdrParams->lutSize = LUT33_SEG_SIZE;
        pHDREngine->isHdr33LutSizeEnabled = true;
        VP_RENDER_NORMALMESSAGE("3DLut table is used 33 lutsize.");
    }
    else
    {
        hdrParams->lutSize = LUT65_SEG_SIZE;
        pHDREngine->isHdr33LutSizeEnabled = false;
        VP_RENDER_NORMALMESSAGE("3DLut table is used 65 lutsize.");
    }

    pHDREngine->is1K1DLutSurfaceInUse = m_hwCaps.m_rules.is1K1DLutSurfaceInUse;
    if (hdrParams->external3DLutParams && userFeatureControl->IsExternal3DLutSupport())
    {
        hdrParams->stage        = HDR_STAGE_VEBOX_EXTERNAL_3DLUT;
        pHDREngine->bEnabled    = 1;
        pHDREngine->VeboxNeeded = 1;
        if (hdrParams->formatOutput == Format_A8B8G8R8 || hdrParams->formatOutput == Format_A8R8G8B8)
        {
            pHDREngine->VeboxARGBOut = 1;
        }
        else if (hdrParams->formatOutput == Format_B10G10R10A2 || hdrParams->formatOutput == Format_R10G10B10A2)
        {
            pHDREngine->VeboxARGB10bitOutput = 1;
        }
        VP_PUBLIC_NORMALMESSAGE("3DLUT table setup by API, use HDR_STAGE_VEBOX_EXTERNAL_3DLUT.");
        return MOS_STATUS_SUCCESS;
    }
    else if (Is3DLutKernelSupported())
    {
        if (userFeatureControl->Is3DLutKernelOnly())
        {
            hdrParams->is3DLutKernelOnly = true;
        }

        if (hdrParams->uiMaxContentLevelLum != m_savedMaxCLL || hdrParams->uiMaxDisplayLum != m_savedMaxDLL ||
            hdrParams->hdrMode != m_savedHdrMode)
        {
            m_savedMaxCLL = hdrParams->uiMaxContentLevelLum;
            m_savedMaxDLL = hdrParams->uiMaxDisplayLum;
            m_savedHdrMode = hdrParams->hdrMode;

            hdrParams->stage         = HDR_STAGE_3DLUT_KERNEL;
            pHDREngine->bEnabled     = 1;
            pHDREngine->isolated     = 1;
            pHDREngine->RenderNeeded = 1;
            hdrFilter->SetRenderTargetType(RenderTargetType::RenderTargetTypeParameter);
            VP_PUBLIC_NORMALMESSAGE("HDR_STAGE_3DLUT_KERNEL");
            // For next stage, will be updated during UpdateFeaturePipe.
        }
        else
        {
            hdrParams->stage         = HDR_STAGE_VEBOX_3DLUT_NO_UPDATE;
            pHDREngine->bEnabled     = 1;
            pHDREngine->VeboxNeeded  = 1;
            hdrFilter->SetRenderTargetType(RenderTargetType::RenderTargetTypeSurface);
            if (hdrParams->formatOutput == Format_A8B8G8R8 || hdrParams->formatOutput == Format_A8R8G8B8)
            {
                pHDREngine->VeboxARGBOut = 1;
            }
            else if (hdrParams->formatOutput == Format_B10G10R10A2 || hdrParams->formatOutput == Format_R10G10B10A2)
            {
                pHDREngine->VeboxARGB10bitOutput = 1;
            }
            VP_PUBLIC_NORMALMESSAGE("HDR_STAGE_VEBOX_3DLUT_NO_UPDATE");
        }
    }
    else
    {
        hdrParams->stage        = HDR_STAGE_DEFAULT;
        pHDREngine->bEnabled    = 1;
        pHDREngine->VeboxNeeded = 1;
        if (hdrParams->formatOutput == Format_A8B8G8R8 || hdrParams->formatOutput == Format_A8R8G8B8)
        {
            pHDREngine->VeboxARGBOut = 1;
        }
        else if (hdrParams->formatOutput == Format_B10G10R10A2 || hdrParams->formatOutput == Format_R10G10B10A2)
        {
            pHDREngine->VeboxARGB10bitOutput = 1;
        }
        VP_PUBLIC_NORMALMESSAGE("HDR_STAGE_DEFAULT");
    }

    PrintFeatureExecutionCaps(__FUNCTION__, *pHDREngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetColorFillExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterColorFill* filter = dynamic_cast<SwFilterColorFill*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(filter);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);

    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableSfc = userFeatureControl->IsSfcDisabled();
    FeatureParamColorFill& params = filter->GetSwFilterParams();
    VP_EngineEntry& engine = filter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = params.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (engine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("ColorFill Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, engine);
        return MOS_STATUS_SUCCESS;
    }

    engine.bEnabled = 1;
    engine.RenderNeeded = 1;
    engine.fcSupported = 1;
    // For disableSfc case, sfc will be filtered in SwFilterColorFill::GetCombinedFilterEngineCaps
    // with scaling setting.
    engine.SfcNeeded = !disableSfc;    // For SFC, the parameter in scaling is used.

    PrintFeatureExecutionCaps(__FUNCTION__, engine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetAlphaExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterAlpha* filter = dynamic_cast<SwFilterAlpha*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(filter);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface());
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetHwInterface()->m_userFeatureControl);

    auto userFeatureControl = m_vpInterface.GetHwInterface()->m_userFeatureControl;
    bool disableSfc = userFeatureControl->IsSfcDisabled();
    FeatureParamAlpha& params = filter->GetSwFilterParams();
    VP_EngineEntry& engine = filter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = params.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (engine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Alpha Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, engine);
        return MOS_STATUS_SUCCESS;
    }
    engine.bEnabled = 1;
    engine.RenderNeeded = 1;
    engine.fcSupported = 1;

    // For Vebox, the alpha parameter in csc is used.
    engine.VeboxNeeded = IsAlphaSettingSupportedByVebox(params.formatInput, params.formatOutput, params.compAlpha);

    if (disableSfc)
    {
        engine.SfcNeeded = false;
    }
    else
    {
        // For SFC, the alpha parameter in scaling is used.
        engine.SfcNeeded = IsAlphaSettingSupportedBySfc(params.formatInput, params.formatOutput, params.compAlpha);
    }

    PrintFeatureExecutionCaps(__FUNCTION__, engine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetLumakeyExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterLumakey* filter = dynamic_cast<SwFilterLumakey*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(filter);

    FeatureParamLumakey& params = filter->GetSwFilterParams();
    VP_EngineEntry& engine = filter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = params.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (engine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Lumakey Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, engine);
        return MOS_STATUS_SUCCESS;
    }

    engine.bEnabled = 1;
    engine.RenderNeeded = 1;
    engine.fcSupported = 1;

    PrintFeatureExecutionCaps(__FUNCTION__, engine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetBlendingExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterBlending* filter = dynamic_cast<SwFilterBlending*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(filter);

    FeatureParamBlending& params = filter->GetSwFilterParams();
    VP_EngineEntry& engine = filter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = params.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (engine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Blending Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, engine);
        return MOS_STATUS_SUCCESS;
    }

    engine.bEnabled = 1;
    engine.RenderNeeded = 1;
    engine.fcSupported = 1;

    PrintFeatureExecutionCaps(__FUNCTION__, engine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetCgcExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterCgc* cgcFilter = dynamic_cast<SwFilterCgc*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(cgcFilter);

    FeatureParamCgc& cgcParams = cgcFilter->GetSwFilterParams();
    VP_EngineEntry& cgcEngine = cgcFilter->GetFilterEngineCaps();
    MOS_FORMAT inputformat = cgcParams.formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (cgcEngine.value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("CGC Feature Already been processed, Skip further process");
        PrintFeatureExecutionCaps(__FUNCTION__, cgcEngine);
        return MOS_STATUS_SUCCESS;
    }

    if (m_hwCaps.m_veboxHwEntry[inputformat].inputSupported &&
        m_hwCaps.m_veboxHwEntry[inputformat].iecp           &&
        m_hwCaps.m_veboxHwEntry[inputformat].cgcSupported)
    {
        cgcEngine.bEnabled         = 1;
        cgcEngine.VeboxNeeded      = 1;
        cgcEngine.VeboxIECPNeeded  = 1;
        cgcEngine.bt2020ToRGB      = cgcParams.bBt2020ToRGB;
    }

    PrintFeatureExecutionCaps(__FUNCTION__, cgcEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(feature);

    VP_EngineEntry &defaultEngine = feature->GetFilterEngineCaps();
    defaultEngine.value = 0;

    PrintFeatureExecutionCaps(__FUNCTION__, defaultEngine);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::InitExecuteCaps(VP_EXECUTE_CAPS &caps, VP_EngineEntry &engineCapsInputPipe, VP_EngineEntry &engineCapsOutputPipe)
{
    caps.value = 0;

    if (0 == engineCapsInputPipe.value)
    {
        caps.bOutputPipeFeatureInuse = engineCapsOutputPipe.bEnabled;
        // For color fill w/o input surface case, engineCapsOutputPipe.fcOnlyFeatureExists should be set.
        if (0 == engineCapsOutputPipe.value || engineCapsOutputPipe.nonFcFeatureExists || !engineCapsOutputPipe.fcOnlyFeatureExists)
        {
            caps.bVebox = true;
            caps.bIECP = engineCapsOutputPipe.VeboxIECPNeeded;
            caps.bSFC = engineCapsOutputPipe.nonVeboxFeatureExists;
        }
        else
        {
            caps.bRender = 1;
            caps.bComposite = 1;
        }
    }
    else if (engineCapsInputPipe.isolated)
    {
        caps.bTemperalInputInuse = engineCapsInputPipe.bTemperalInputInuse;
        if (engineCapsInputPipe.VeboxNeeded != 0 || engineCapsInputPipe.SfcNeeded != 0)
        {
            caps.bVebox = true;
            caps.bIECP = engineCapsInputPipe.VeboxIECPNeeded;
            caps.bSFC = engineCapsInputPipe.SfcNeeded != 0;
        }
        else if (engineCapsInputPipe.RenderNeeded)
        {
            caps.bRender = 1;

            if (engineCapsInputPipe.isOutputPipeNeeded)
            {
                caps.bOutputPipeFeatureInuse = true;
            }
        }
        else
        {
            // No valid engine selected.
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }
    else if (engineCapsInputPipe.hdrKernelNeeded)
    {
        caps.bRender = 1;
        caps.bRenderHdr = engineCapsInputPipe.hdrKernelNeeded;
        caps.bOutputPipeFeatureInuse = true;
    }
    else if (engineCapsInputPipe.nonFcFeatureExists)
    {
        VP_EngineEntry engineCaps = engineCapsInputPipe;
        bool multiPassNeeded = false;
        if (!engineCaps.fcOnlyFeatureExists && !engineCapsOutputPipe.fcOnlyFeatureExists &&
            !engineCaps.multiPassNeeded)
        {
            caps.bOutputPipeFeatureInuse = true;
            engineCaps.value |= engineCapsOutputPipe.value;
        }
        caps.bVebox = true;
        caps.bIECP = engineCaps.VeboxIECPNeeded;
        caps.bDiProcess2ndField = engineCaps.diProcess2ndField;
        caps.bTemperalInputInuse = engineCaps.bTemperalInputInuse;
        caps.b1K1DLutInUse       = engineCaps.is1K1DLutSurfaceInUse;
        caps.bDemosaicInUse      = engineCaps.isBayerInputInUse;
        
        if (engineCaps.fcOnlyFeatureExists)
        {
            // For vebox/sfc+render case, use 2nd workload (render) to do csc for better performance
            // in most VP common cases, e.g. NV12->RGB, to save the memory bandwidth.
            caps.bForceCscToRender     = true;
            // Force procamp to render if both enable Lumaykey and procamp on the same layer
            // Lumaykey should be top-priority
            if (engineCaps.outputWithLumaKey)
            {
                caps.bForceProcampToRender = true;
            }
            else
            {
                // For vebox/sfc+render case, use 2nd workload (render) to do Procamp,
                // especially for the scenario including Lumakey feature, which will ensure the Procamp can be done after Lumakey.
                caps.bForceProcampToRender = false;
            }
            // For vebox + render with features, which can be done on both sfc and render, 
            // and sfc is not must have, sfc should not be selected and those features should be done on render.
            caps.bSFC = engineCaps.nonVeboxFeatureExists && engineCaps.sfcOnlyFeatureExists;
            // For OCL FC not supported formats, fallback to legacy FC
            caps.bFallbackLegacyFC = engineCaps.forceLegacyFC;

        }
        else
        {
            caps.bSFC = !engineCapsOutputPipe.sfcNotSupported && engineCaps.nonVeboxFeatureExists;
        }
    }
    else if (engineCapsInputPipe.forceBypassWorkload)
    {
        caps.bVebox               = 0;
        caps.bSFC                 = 0;
        caps.bRender              = 0;
        caps.forceBypassWorkload  = 1;
    }
    else
    {
        if (!engineCapsInputPipe.fcSupported)
        {
            // Should be only fc feature here.
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        VP_EngineEntry engineCaps = engineCapsInputPipe;
        engineCaps.value |= engineCapsOutputPipe.value;
        caps.bOutputPipeFeatureInuse = true;
        // Fc cases.
        if (!engineCaps.fcOnlyFeatureExists && !engineCaps.veboxNotSupported)
        {
            // If all feature can support both vebox/sfc and fc.
            caps.bVebox = true;
            caps.bIECP = engineCapsInputPipe.VeboxIECPNeeded;
            caps.bSFC = engineCapsInputPipe.nonVeboxFeatureExists;
        }
        else
        {
            caps.bRender = 1;
            caps.bComposite = 1;
        }

        caps.bDiProcess2ndField = engineCaps.diProcess2ndField;
        caps.bTemperalInputInuse = engineCaps.bTemperalInputInuse;
        caps.b1K1DLutInUse       = engineCaps.is1K1DLutSurfaceInUse;
        caps.bFallbackLegacyFC   = engineCaps.forceLegacyFC;
    }
    if (caps.bSFC && engineCapsInputPipe.enableSFCLinearOutputByTileConvert)
    {
        caps.enableSFCLinearOutputByTileConvert = engineCapsInputPipe.enableSFCLinearOutputByTileConvert;
    }
    VP_PUBLIC_NORMALMESSAGE("Execute Caps, value 0x%llx (bVebox %d, bSFC %d, bRender %d, bComposite %d, bOutputPipeFeatureInuse %d, bIECP %d, bForceCscToRender %d, bDiProcess2ndField %d)",
        caps.value, caps.bVebox, caps.bSFC, caps.bRender, caps.bComposite, caps.bOutputPipeFeatureInuse, caps.bIECP,
        caps.bForceCscToRender, caps.bDiProcess2ndField);
    PrintFeatureExecutionCaps("engineCapsInputPipe", engineCapsInputPipe);
    PrintFeatureExecutionCaps("engineCapsOutputPipe", engineCapsOutputPipe);
    
    MT_LOG7(MT_VP_HAL_POLICY_INIT_EXECCAPS, MT_NORMAL, MT_VP_HAL_EXECCAPS, (int64_t)caps.value, MT_VP_HAL_EXECCAPS_VE, (int64_t)caps.bVebox, MT_VP_HAL_EXECCAPS_SFC, (int64_t)caps.bSFC, MT_VP_HAL_EXECCAPS_RENDER,
        (int64_t)caps.bRender, MT_VP_HAL_EXECCAPS_COMP, (int64_t)caps.bComposite, MT_VP_HAL_EXECCAPS_OUTPIPE_FTRINUSE, (int64_t)caps.bOutputPipeFeatureInuse, MT_VP_HAL_EXECCAPS_IECP, (int64_t)caps.bIECP);
    MT_LOG7(MT_VP_HAL_POLICY_GET_INPIPECAPS, MT_NORMAL, MT_VP_HAL_ENGINECAPS, (int64_t)engineCapsInputPipe.value, MT_VP_HAL_ENGINECAPS_EN, (int64_t)engineCapsInputPipe.bEnabled, MT_VP_HAL_ENGINECAPS_VE_NEEDED,
        (int64_t)engineCapsInputPipe.VeboxNeeded, MT_VP_HAL_ENGINECAPS_SFC_NEEDED, (int64_t)engineCapsInputPipe.SfcNeeded, MT_VP_HAL_ENGINECAPS_RENDER_NEEDED, (int64_t)engineCapsInputPipe.RenderNeeded, MT_VP_HAL_ENGINECAPS_FC_SUPPORT, (int64_t)engineCapsInputPipe.fcSupported, MT_VP_HAL_ENGINECAPS_ISOLATED, (int64_t)engineCapsInputPipe.isolated);
    MT_LOG7(MT_VP_HAL_POLICY_GET_OUTPIPECAPS, MT_NORMAL, MT_VP_HAL_ENGINECAPS, (int64_t)engineCapsOutputPipe.value, MT_VP_HAL_ENGINECAPS_EN, (int64_t)engineCapsOutputPipe.bEnabled, MT_VP_HAL_ENGINECAPS_VE_NEEDED, (int64_t)engineCapsOutputPipe.VeboxNeeded, MT_VP_HAL_ENGINECAPS_SFC_NEEDED, (int64_t)engineCapsOutputPipe.SfcNeeded, MT_VP_HAL_ENGINECAPS_RENDER_NEEDED, (int64_t)engineCapsOutputPipe.RenderNeeded,
        MT_VP_HAL_ENGINECAPS_FC_SUPPORT, (int64_t)engineCapsOutputPipe.fcSupported, MT_VP_HAL_ENGINECAPS_ISOLATED, (int64_t)engineCapsOutputPipe.isolated);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetOutputPipeEngineCaps(SwFilterPipe& featurePipe, VP_EngineEntry &engineCapsOutputPipe, SwFilterSubPipe *inputPipeSelected)
{
    SwFilterSubPipe *featureSubPipe = featurePipe.GetSwFilterSubPipe(false, 0);
    VP_PUBLIC_CHK_NULL_RETURN(featureSubPipe);

    engineCapsOutputPipe.value = 0;

    for (auto featureType : m_featurePool)
    {
        SwFilter *swFilter = featureSubPipe->GetSwFilter(featureType);

        if (nullptr == swFilter)
        {
            continue;
        }

        VP_EngineEntry engineCaps = swFilter->GetCombinedFilterEngineCaps(inputPipeSelected);

        if (!engineCaps.bEnabled)
        {
            continue;
        }

        if (engineCaps.isolated || !engineCaps.RenderNeeded || !engineCaps.fcSupported)
        {
            // No support for non-fc supported feature in current stage.
            // Will add it if needed.
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }

        if (!engineCaps.SfcNeeded && !engineCaps.VeboxNeeded && !engineCaps.bypassIfVeboxSfcInUse)
        {
            engineCapsOutputPipe.fcOnlyFeatureExists = true;
        }
        engineCapsOutputPipe.value |= engineCaps.value;
        engineCapsOutputPipe.nonVeboxFeatureExists |= (!engineCaps.VeboxNeeded && !engineCaps.bypassIfVeboxSfcInUse);
    }

    PrintFeatureExecutionCaps(__FUNCTION__, engineCapsOutputPipe);
    MT_LOG7(MT_VP_HAL_POLICY_GET_OUTPIPECAPS, MT_NORMAL, MT_VP_HAL_ENGINECAPS, (int64_t)engineCapsOutputPipe.value, MT_VP_HAL_ENGINECAPS_EN, (int64_t)engineCapsOutputPipe.bEnabled,
        MT_VP_HAL_ENGINECAPS_VE_NEEDED, (int64_t)engineCapsOutputPipe.VeboxNeeded, MT_VP_HAL_ENGINECAPS_SFC_NEEDED, (int64_t)engineCapsOutputPipe.SfcNeeded,
        MT_VP_HAL_ENGINECAPS_RENDER_NEEDED, (int64_t)engineCapsOutputPipe.RenderNeeded, MT_VP_HAL_ENGINECAPS_FC_SUPPORT, (int64_t)engineCapsOutputPipe.fcSupported,
        MT_VP_HAL_ENGINECAPS_ISOLATED, (int64_t)engineCapsOutputPipe.isolated);

    return MOS_STATUS_SUCCESS;
}

bool Policy::IsExcludedFeatureForHdr(FeatureType feature)
{
    return (FeatureTypeTcc == feature ||
            FeatureTypeSte == feature ||
            FeatureTypeProcamp == feature);
}

bool Policy::IsIsolateFeatureOutputPipeNeeded(SwFilterSubPipe *featureSubPipe, SwFilter *swFilter)
{
    if (RenderTargetTypeSurface == swFilter->GetRenderTargetType())
    {
        bool isIsolateFeatureOutputPipeNeeded = true;
        for (auto featureType : m_featurePool)
        {
            SwFilter      *curSwFilter = featureSubPipe->GetSwFilter(featureType);
            if (nullptr != curSwFilter)
            {
                VP_EngineEntry caps = curSwFilter->GetFilterEngineCaps();
                if (caps.bEnabled == true && featureType != swFilter->GetFeatureType())
                {
                    auto renderTargetType = curSwFilter->GetRenderTargetType();
                    if (renderTargetType == RenderTargetTypeSurface)
                    {
                        isIsolateFeatureOutputPipeNeeded = false;
                        break;
                    }
                }
            }
        }
        return isIsolateFeatureOutputPipeNeeded;
    }
    else
    {
        return false;
    }
}

MOS_STATUS Policy::GetInputPipeEngineCaps(SwFilterPipe& featurePipe, VP_EngineEntry &engineCapsInputPipe,
                                        SwFilterSubPipe *&singlePipeSelected, bool &isSingleSubPipe, uint32_t &selectedPipeIndex)
{
    // Priority for selecting features to be processed in current workload.
    // 1. Features with isolated flag. One isolated feature can only be processed without any other features involved.
    // 2. Features without fc supported flag.
    // 3. For single layer, select vebox, if all vebox == 1
    //                      select sfc, if all sfc/vebox == 1 and sfc == 1/vebox == 0 exists
    //    For multi layer, select FC
    // Add support for ordered pipe later.
    isSingleSubPipe = featurePipe.GetSurfaceCount(true) <= 1;
    singlePipeSelected = isSingleSubPipe ? featurePipe.GetSwFilterSubPipe(true, 0) : nullptr;
    selectedPipeIndex = 0;

    VP_EngineEntry engineCapsIsolated = {};     // Input pipe engine caps for isolated feature exists case.
    VP_EngineEntry engineCapsForVeboxSfc = {};  // Input pipe engine caps for non-fc feature exists case.
    VP_EngineEntry engineCapsForFc = {};        // Input pipe engine caps for fc supported by all features cases.
    VP_EngineEntry engineCapsForHdrKernel = {0};  // Input pipe engine caps for hdr kernel supported by all features cases.
    for (uint32_t pipeIndex = 0; pipeIndex < featurePipe.GetSurfaceCount(true); ++pipeIndex)
    {
        SwFilterSubPipe *featureSubPipe = featurePipe.GetSwFilterSubPipe(true, pipeIndex);
        VP_PUBLIC_CHK_NULL_RETURN(featureSubPipe);

        bool isSfcNeeded = false;
        engineCapsForVeboxSfc.value = 0;

        for (auto featureType : m_featurePool)
        {
            SwFilter *swFilter = featureSubPipe->GetSwFilter(featureType);
            if (nullptr == swFilter)
            {
                continue;
            }

            VP_EngineEntry &engineCaps = swFilter->GetFilterEngineCaps();

            if (!engineCaps.bEnabled)
            {
                // Some features need to be bypassed with requirement on workload, e.g. 2nd field for DI.
                if (engineCaps.bypassVeboxFeatures || engineCaps.diProcess2ndField)
                {
                    isSingleSubPipe = true;
                    selectedPipeIndex = pipeIndex;
                    singlePipeSelected = featureSubPipe;
                    engineCapsForVeboxSfc.value |= engineCaps.value;
                    engineCapsForFc.value |= engineCaps.value;
                }
                if (engineCaps.sfcNotSupported)
                {
                    // sfc cannot be selected. Resolution limit is checked with scaling filter, even scaling
                    // feature itself not being enabled.
                    engineCapsForVeboxSfc.sfcNotSupported = engineCaps.sfcNotSupported;
                    engineCapsForFc.sfcNotSupported = engineCaps.sfcNotSupported;
                    VP_PUBLIC_NORMALMESSAGE("sfcNotSupported flag is set.");
                }
                if (engineCaps.veboxNotSupported)
                {
                    // vebox cannot be selected. Resolution limit is checked with scaling filter, even scaling
                    // feature itself not being enabled.
                    engineCapsForVeboxSfc.veboxNotSupported = engineCaps.veboxNotSupported;
                    engineCapsForFc.veboxNotSupported       = engineCaps.veboxNotSupported;
                    VP_PUBLIC_NORMALMESSAGE("veboxNotSupported flag is set.");
                }
                if (engineCaps.forceBypassWorkload)
                {
                    engineCapsInputPipe.forceBypassWorkload = engineCaps.forceBypassWorkload;
                    VP_PUBLIC_NORMALMESSAGE("Set engineCapsInputPipe forceDisableForVebox true.");
                }
                continue;
            }

            if (engineCaps.isolated)
            {
                // 1. Process feature with isolated flag firstly.
                isSingleSubPipe = true;
                selectedPipeIndex = pipeIndex;
                singlePipeSelected = featureSubPipe;
                engineCapsIsolated = engineCaps;
                engineCapsIsolated.isOutputPipeNeeded = IsIsolateFeatureOutputPipeNeeded(featureSubPipe, swFilter);
                break;
            }
            else if (!engineCaps.fcSupported)
            {
                // 2. Process non-fc features by vebox/sfc.
                if (engineCaps.RenderNeeded && !engineCaps.hdrKernelNeeded)
                {
                    // Non-FC render feature should be isolated.
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
                }

                if (engineCaps.hdrKernelSupported)
                {
                    engineCapsForHdrKernel.value |= engineCaps.value;
                }

                if (engineCaps.hdrKernelNeeded)
                {
                    VP_PUBLIC_NORMALMESSAGE("HDR kernel is needed.");
                }
                else
                {
                    if (!engineCaps.SfcNeeded && !engineCaps.VeboxNeeded)
                    {
                        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER)
                    }
                    if (engineCaps.sfcNotSupported)
                    {
                        // sfc cannot be selected. Resolution limit is checked with scaling filter, even scaling
                        // feature itself not being enabled.
                        engineCapsForVeboxSfc.sfcNotSupported = engineCaps.sfcNotSupported;
                        engineCapsForFc.sfcNotSupported       = engineCaps.sfcNotSupported;
                        VP_PUBLIC_NORMALMESSAGE("sfcNotSupported flag is set.");
                    }

                    if ((engineCaps.SfcNeeded && !engineCaps.RenderNeeded && !engineCaps.VeboxNeeded))
                    {
                        engineCapsForVeboxSfc.sfcOnlyFeatureExists = 1;
                    }

                    isSingleSubPipe    = true;
                    selectedPipeIndex  = pipeIndex;
                    singlePipeSelected = featureSubPipe;
                    engineCapsForVeboxSfc.value |= engineCaps.value;
                    engineCapsForVeboxSfc.nonFcFeatureExists = true;
                    engineCapsForVeboxSfc.nonVeboxFeatureExists |= !engineCaps.VeboxNeeded;

                    SwFilter *lumakey          = featureSubPipe->GetSwFilter(FeatureTypeLumakey);
                    if (lumakey && lumakey->GetFilterEngineCaps().bEnabled)
                    {
                        engineCapsForVeboxSfc.outputWithLumaKey = true;
                        VP_PUBLIC_NORMALMESSAGE("outputWithLumaKey flag is set.");
                    }
                    else
                    {
                        engineCapsForVeboxSfc.outputWithLumaKey = false;
                    }
                }
            }
            else
            {
                if (engineCaps.hdrKernelSupported)
                {
                    engineCapsForHdrKernel.value |= engineCaps.value;
                }

                if (engineCaps.hdrKernelNeeded)
                {
                    VP_PUBLIC_ASSERTMESSAGE("hdrKernelNeeded and fcSupported should not be set at same time.");
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER)
                }

                // 3. Process features support FC.
                if (engineCaps.SfcNeeded || engineCaps.VeboxNeeded)
                {
                    engineCapsForVeboxSfc.value |= engineCaps.value;
                    engineCapsForVeboxSfc.nonVeboxFeatureExists |= !engineCaps.VeboxNeeded;
                }
                else
                {
                    // Set fcOnlyFeatureExists flag for engineCapsForVeboxSfc to indicate that
                    // not all features in input pipe can be processed by vebox/sfc and
                    // features in output pipe cannot be combined to vebox/sfc workload.
                    engineCapsForVeboxSfc.fcOnlyFeatureExists = true;
                    engineCapsForFc.fcOnlyFeatureExists = true;
                }
                if (engineCaps.sfcNotSupported)
                {
                    // sfc cannot be selected. Resolution limit is checked with scaling filter, even scaling
                    // feature itself not being enabled.
                    engineCapsForVeboxSfc.sfcNotSupported = engineCaps.sfcNotSupported;
                    engineCapsForFc.sfcNotSupported       = engineCaps.sfcNotSupported;
                    VP_PUBLIC_NORMALMESSAGE("sfcNotSupported flag is set.");
                }

                engineCapsForFc.value |= engineCaps.value;
                engineCapsForFc.nonVeboxFeatureExists |= !engineCaps.VeboxNeeded;
            }
        }

        if (isSingleSubPipe)
        {
            break;
        }
    }

    // For multi-layer case or color fill case, force to set fcOnlyFeatureExists flag.
    engineCapsForFc.fcOnlyFeatureExists = engineCapsForFc.fcOnlyFeatureExists ||
                                        featurePipe.GetSurfaceCount(true) > 1 ||
                                        featurePipe.GetSurfaceCount(true) == 0 ||
                                        engineCapsForFc.nonVeboxFeatureExists && engineCapsForFc.sfcNotSupported;

    if (engineCapsForVeboxSfc.nonVeboxFeatureExists && engineCapsForVeboxSfc.sfcNotSupported)
    {
        VP_PUBLIC_NORMALMESSAGE("Clear nonVeboxFeatureExists flag to avoid sfc being selected, since sfcNotSupported == 1.");
        engineCapsForVeboxSfc.nonVeboxFeatureExists = 0;
    }

    // If want to disable vebox/sfc output to output surface with color fill directly for multilayer case,
    // fcOnlyFeatureExists need be set for vebox sfc for multi layer case here.
    engineCapsForVeboxSfc.fcOnlyFeatureExists = engineCapsForFc.fcOnlyFeatureExists;

    if (engineCapsIsolated.isolated)
    {
        VP_PUBLIC_NORMALMESSAGE("engineCapsIsolated selected.");
        engineCapsInputPipe = engineCapsIsolated;
    }
    else if (engineCapsForHdrKernel.hdrKernelNeeded)
    {
        VP_PUBLIC_NORMALMESSAGE("engineCapsForHdrKernel selected.");
        engineCapsInputPipe = engineCapsForHdrKernel;
    }
    else if (engineCapsForVeboxSfc.nonFcFeatureExists)
    {
        VP_PUBLIC_NORMALMESSAGE("engineCapsForVeboxSfc selected.");
        engineCapsInputPipe = engineCapsForVeboxSfc;
    }
    else if (engineCapsInputPipe.forceBypassWorkload)
    {
        VP_PUBLIC_NORMALMESSAGE("Still use engineCapsInputPipe forceBypassWorkload set true.");
    }
    else
    {
        VP_PUBLIC_NORMALMESSAGE("engineCapsForFc selected.");
        if (0 == engineCapsForFc.bEnabled)
        {
            engineCapsForFc.fcSupported = true;
            // Decision on choosing between vebox/sfc and render will be made in InitExecuteCaps.
            VP_PUBLIC_NORMALMESSAGE("Surface copy with no feature enabled. Force set fcSupported flag.");
        }
        engineCapsInputPipe = engineCapsForFc;
    }

    PrintFeatureExecutionCaps(__FUNCTION__, engineCapsInputPipe);
    MT_LOG7(MT_VP_HAL_POLICY_GET_INPIPECAPS, MT_NORMAL, MT_VP_HAL_ENGINECAPS, (int64_t)engineCapsInputPipe.value,
        MT_VP_HAL_ENGINECAPS_EN, (int64_t)engineCapsInputPipe.bEnabled, MT_VP_HAL_ENGINECAPS_VE_NEEDED, (int64_t)engineCapsInputPipe.VeboxNeeded,
        MT_VP_HAL_ENGINECAPS_SFC_NEEDED, (int64_t)engineCapsInputPipe.SfcNeeded, MT_VP_HAL_ENGINECAPS_RENDER_NEEDED, (int64_t)engineCapsInputPipe.RenderNeeded,
        MT_VP_HAL_ENGINECAPS_FC_SUPPORT, (int64_t)engineCapsInputPipe.fcSupported, MT_VP_HAL_ENGINECAPS_ISOLATED, (int64_t)engineCapsInputPipe.isolated);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BypassVeboxFeatures(SwFilterSubPipe *featureSubPipe, VP_EngineEntry &engineCaps)
{
    VP_PUBLIC_CHK_NULL_RETURN(featureSubPipe);

    for (auto filterID : m_featurePool)
    {
        SwFilter *feature = featureSubPipe->GetSwFilter(FeatureType(filterID));

        if (nullptr == feature)
        {
            continue;
        }

        if (feature->GetFilterEngineCaps().VeboxNeeded)
        {
            // Disable vebox features and avoid it run into render path.
            feature->GetFilterEngineCaps().VeboxNeeded = 0;
            feature->GetFilterEngineCaps().RenderNeeded = 0;
            feature->GetFilterEngineCaps().bEnabled = feature->GetFilterEngineCaps().SfcNeeded;
        }
    }

    engineCaps.nonVeboxFeatureExists = true;
    VP_PUBLIC_NORMALMESSAGE("Set nonVeboxFeatureExists to 1 for BypassVeboxFeatures.");

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecuteCaps(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS &caps, VP_EngineEntry &engineCapsInputPipe, VP_EngineEntry &engineCapsOutputPipe,
                                    bool &isSingleSubPipe, uint32_t &selectedPipeIndex)
{
    SwFilterSubPipe *singlePipeSelected = nullptr;

    caps.value = 0;
    engineCapsOutputPipe.value = 0;
    engineCapsInputPipe.value = 0;
    isSingleSubPipe = false;
    selectedPipeIndex = 0;

    VP_PUBLIC_CHK_STATUS_RETURN(GetInputPipeEngineCaps(featurePipe, engineCapsInputPipe,
                                                    singlePipeSelected, isSingleSubPipe, selectedPipeIndex));
    VP_PUBLIC_CHK_STATUS_RETURN(GetOutputPipeEngineCaps(featurePipe, engineCapsOutputPipe, singlePipeSelected));

    if (engineCapsInputPipe.bypassVeboxFeatures)
    {
        if (engineCapsInputPipe.isolated)
        {
            // isolated feature case should not come here.
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        VP_PUBLIC_CHK_STATUS_RETURN(BypassVeboxFeatures(singlePipeSelected, engineCapsInputPipe));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(InitExecuteCaps(caps, engineCapsInputPipe, engineCapsOutputPipe));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildFilters(SwFilterPipe& featurePipe, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_EngineEntry engineCaps = {};
    VP_EXECUTE_CAPS caps = {};
    VP_EngineEntry engineCapsInputPipe = {};
    VP_EngineEntry engineCapsOutputPipe = {};
    bool isSingleSubPipe = false;
    uint32_t selectedPipeIndex = 0;

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteCaps(featurePipe, caps, engineCapsInputPipe, engineCapsOutputPipe, isSingleSubPipe, selectedPipeIndex));

    std::vector<int> layerIndexes;
    VP_PUBLIC_CHK_STATUS_RETURN(LayerSelectForProcess(layerIndexes, featurePipe, isSingleSubPipe, selectedPipeIndex, caps));

    if (IsVeboxSecurePathEnabled(featurePipe, caps))
    {
        // Process Vebox Secure workload
        VP_PUBLIC_CHK_STATUS_RETURN(BuildVeboxSecureFilters(featurePipe, caps, params));

        VP_PUBLIC_CHK_STATUS_RETURN(SetupFilterResource(featurePipe, layerIndexes, caps, params));

        VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteHwFilter(caps, params));
        return MOS_STATUS_SUCCESS;
    }

    // Set feature types with engine for selected features
    VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureTypeWithEngine(layerIndexes, featurePipe, caps, engineCapsInputPipe.isolated, caps.bOutputPipeFeatureInuse/*engineCapsOutputPipe.bEnabled*/));

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteFilter(featurePipe, layerIndexes, caps, params));
    VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.ResetSecureFlag());

    /* Place Holder for Resource Manager to manage intermedia surface or HW needed surface in policy*/

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::FilterFeatureCombination(SwFilterPipe &swFilterPipe, bool isInputPipe, uint32_t index, VP_EngineEntry &engineCapsCombined, VP_EngineEntry &engineCapsCombinedAllPipes)
{
    VP_FUNC_CALL();

    SwFilterSubPipe *pipe = nullptr;
    pipe                  = swFilterPipe.GetSwFilterSubPipe(isInputPipe, index);
    VP_PUBLIC_CHK_NULL_RETURN(pipe);

    // Filter out vebox/sfc only features.
    if (engineCapsCombined.veboxNotSupported)
    {
        for (auto filterID : m_featurePool)
        {
            auto feature = pipe->GetSwFilter(FeatureType(filterID));
            if (feature && feature->GetFilterEngineCaps().bEnabled &&
                (feature->GetFilterEngineCaps().VeboxNeeded ||
                feature->GetFilterEngineCaps().SfcNeeded) &&
                !feature->GetFilterEngineCaps().RenderNeeded)
            {
                feature->GetFilterEngineCaps().bEnabled = 0;
                feature->GetFilterEngineCaps().VeboxNeeded = 0;
                feature->GetFilterEngineCaps().SfcNeeded = 0;
                feature->GetFilterEngineCaps().forceEnableForSfc = 0;
                VP_PUBLIC_NORMALMESSAGE("Disable feature 0x%x since vebox cannot be used.", filterID);
                PrintFeatureExecutionCaps("Disable feature since vebox cannot be used", feature->GetFilterEngineCaps());
            }
        }
    }
    else if (engineCapsCombined.forceBypassWorkload)
    {
        for (auto filterID : m_featurePool)
        {
            auto feature = pipe->GetSwFilter(FeatureType(filterID));
            if (feature && feature->GetFilterEngineCaps().bEnabled)
            {
                feature->GetFilterEngineCaps().bEnabled     = 0;
                feature->GetFilterEngineCaps().VeboxNeeded  = 0;
                feature->GetFilterEngineCaps().SfcNeeded    = 0;
                feature->GetFilterEngineCaps().RenderNeeded = 0;
                feature->GetFilterEngineCaps()              = {0};
                feature->GetFilterEngineCaps().forceBypassWorkload = 1;
                VP_PUBLIC_NORMALMESSAGE("Disable feature 0x%x because of  bypass this workload.", filterID);
                PrintFeatureExecutionCaps("Disable feature because of  bypass this workload.", feature->GetFilterEngineCaps());     
            }       
        }
    }

    // For DI on render with scaling case, force to do scaling on render.
    if (engineCapsCombined.SfcNeeded)
    {
        auto di      = pipe->GetSwFilter(FeatureType(FeatureTypeDi));

        if (di && di->GetFilterEngineCaps().bEnabled &&
            !di->GetFilterEngineCaps().VeboxNeeded && di->GetFilterEngineCaps().RenderNeeded)
        {
            for (auto filterID : m_featurePool)
            {
                auto feature = pipe->GetSwFilter(FeatureType(filterID));
                if (nullptr == feature || !feature->GetFilterEngineCaps().bEnabled)
                {
                    continue;
                }
                if (FeatureType(filterID) == FeatureTypeScaling &&
                    feature->GetFilterEngineCaps().SfcNeeded &&
                    !feature->GetFilterEngineCaps().RenderNeeded &&
                    !m_hwCaps.m_rules.isAvsSamplerSupported)
                {
                    feature->GetFilterEngineCaps().SfcNeeded    = 0;
                    feature->GetFilterEngineCaps().RenderNeeded = 1;
                    feature->GetFilterEngineCaps().fcSupported  = 1;
                    PrintFeatureExecutionCaps("Update scaling caps for render DI", feature->GetFilterEngineCaps());
                }
                else if (!feature->GetFilterEngineCaps().VeboxNeeded &&
                         feature->GetFilterEngineCaps().SfcNeeded &&
                         feature->GetFilterEngineCaps().RenderNeeded)
                {
                    feature->GetFilterEngineCaps().SfcNeeded = 0;
                    VP_PUBLIC_NORMALMESSAGE("Force feature 0x%x to render for render DI", filterID);
                    MT_LOG2(MT_VP_HAL_POLICY_FLITER_FTR_COMBINE, MT_NORMAL, MT_VP_HAL_FEATUERTYPE, filterID, MT_VP_HAL_EXECCAPS_FORCE_DI2RENDER, (int64_t)feature->GetFilterEngineCaps().RenderNeeded);
                    auto engineCaps = feature->GetFilterEngineCaps();
                    PrintFeatureExecutionCaps("Force feature to render for render DI", engineCaps);
                    MT_LOG7(MT_VP_HAL_POLICY_FLITER_FTR_COMBINE, MT_NORMAL, MT_VP_HAL_FEATUERTYPE, filterID, MT_VP_HAL_ENGINECAPS, (int64_t)engineCaps.value, MT_VP_HAL_ENGINECAPS_EN, (int64_t)engineCaps.bEnabled,
                        MT_VP_HAL_ENGINECAPS_VE_NEEDED, (int64_t)engineCaps.VeboxNeeded, MT_VP_HAL_ENGINECAPS_SFC_NEEDED, (int64_t)engineCaps.SfcNeeded,
                        MT_VP_HAL_ENGINECAPS_RENDER_NEEDED, (int64_t)engineCaps.RenderNeeded, MT_VP_HAL_ENGINECAPS_FC_SUPPORT, (int64_t)engineCaps.fcSupported);
                }
            }
        }
    }

    if (engineCapsCombinedAllPipes.hdrKernelNeeded)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(FilterFeatureCombinationForHDRKernel(pipe));
    }
    else
    {
        auto hdr = pipe->GetSwFilter(FeatureTypeHdr);

        if (nullptr != hdr)
        {
            for (auto filterID : m_featurePool)
            {
                if (IsExcludedFeatureForHdr(filterID))
                {
                    auto feature = pipe->GetSwFilter(FeatureType(filterID));
                    if (feature && feature->GetFilterEngineCaps().bEnabled)
                    {
                        feature->GetFilterEngineCaps().bEnabled = false;
                        VP_PUBLIC_NORMALMESSAGE("Disable feature 0x%x for HDR.", filterID);
                        PrintFeatureExecutionCaps("Disable feature for HDR", feature->GetFilterEngineCaps());
                    }
                }

                if (filterID == FeatureTypeCsc)
                {
                    SwFilterCsc *feature = (SwFilterCsc *)pipe->GetSwFilter(FeatureType(filterID));
                    if (feature)
                    {
                        auto &params      = feature->GetSwFilterParams();
                        params.pIEFParams = nullptr;
                        PrintFeatureExecutionCaps("Disable IEF for HDR", feature->GetFilterEngineCaps());
                    }
                }
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::FilterFeatureCombinationForHDRKernel(SwFilterSubPipe *pipe)
{
    VP_FUNC_CALL();

    for (auto filterID : m_featurePool)
    {
        auto feature = pipe->GetSwFilter(FeatureType(filterID));
        if (feature && feature->GetFilterEngineCaps().bEnabled &&
            !feature->GetFilterEngineCaps().hdrKernelSupported)
        {
            auto feature = pipe->GetSwFilter(FeatureType(filterID));
            if (feature && feature->GetFilterEngineCaps().bEnabled)
            {
                feature->GetFilterEngineCaps().bEnabled = false;
                VP_PUBLIC_NORMALMESSAGE("Disable feature 0x%x for HDR.", filterID);
                PrintFeatureExecutionCaps("Disable feature for HDR", feature->GetFilterEngineCaps());
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecuteFilter(SwFilterPipe& featurePipe, std::vector<int> &layerIndexes, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    params.Type = EngineTypeInvalid;
    // params.vpExecuteCaps will be assigned in Policy::BuildExecuteHwFilter.
    params.vpExecuteCaps.value = 0;

    VP_PUBLIC_CHK_STATUS_RETURN(SetupExecuteFilter(featurePipe, layerIndexes, caps, params));

    // Build Execute surface needed
    VP_PUBLIC_CHK_STATUS_RETURN(SetupFilterResource(featurePipe, layerIndexes, caps, params));

    VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.Update());
    VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->Update());

    if (featurePipe.IsEmpty())
    {
        caps.lastSubmission = true;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteHwFilter(caps, params));

    if (params.executedFilters)
    {
        params.executedFilters->AddRTLog();
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecuteHwFilter(VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    if (caps.bVebox || caps.bSFC)
    {
        params.Type = caps.bSFC ? EngineTypeVeboxSfc : EngineTypeVebox;
        params.vpExecuteCaps = caps;
        auto it = m_VeboxSfcFeatureHandlers.begin();
        for (; it != m_VeboxSfcFeatureHandlers.end(); ++it)
        {
            if ((*(it->second)).IsFeatureEnabled(caps))
            {
                HwFilterParameter* pHwFilterParam = (*(it->second)).CreateHwFilterParam(caps, *params.executedFilters, m_vpInterface.GetHwInterface());

                if (pHwFilterParam)
                {
                    params.Params.push_back(pHwFilterParam);
                }
                else
                {
                    VP_PUBLIC_ASSERTMESSAGE("Create HW Filter Failed, Return Error");
                    MT_ERR2(MT_VP_HAL_POLICY, MT_ERROR_CODE, MOS_STATUS_NO_SPACE, MT_CODE_LINE, __LINE__);
                    return MOS_STATUS_NO_SPACE;
                }
            }
        }
    }
    else if (caps.bRender)
    {
        params.Type = EngineTypeRender;
        params.vpExecuteCaps = caps;

        auto it = m_RenderFeatureHandlers.begin();
        for (; it != m_RenderFeatureHandlers.end(); ++it)
        {
            if ((*(it->second)).IsFeatureEnabled(caps))
            {
                HwFilterParameter* pHwFilterParam = (*(it->second)).CreateHwFilterParam(caps, *params.executedFilters, m_vpInterface.GetHwInterface());

                if (pHwFilterParam)
                {
                    params.Params.push_back(pHwFilterParam);
                }
                else
                {
                    VP_PUBLIC_ASSERTMESSAGE("Create HW Filter Failed, Return Error");
                    MT_ERR2(MT_VP_HAL_POLICY, MT_ERROR_CODE, MOS_STATUS_NO_SPACE, MT_CODE_LINE, __LINE__);
                    return MOS_STATUS_NO_SPACE;
                }
            }
        }
    }
    else if (caps.forceBypassWorkload)
    {
        VP_PUBLIC_NORMALMESSAGE("No engine is assigned. Skip this process for test usage.");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_SUCCESS);
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("No engine is assigned.");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateFeatureTypeWithEngine(std::vector<int> &layerIndexes, SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps,
                                            bool isolatedFeatureSelected, bool outputPipeNeeded)
{
    int inputSurfCount = featurePipe.GetSurfaceCount(true);
    int outputSurfCount = featurePipe.GetSurfaceCount(false);
    SwFilterSubPipe* featureSubPipe = nullptr;

    for (uint32_t i = 0; i < layerIndexes.size(); ++i)
    {
        featureSubPipe = featurePipe.GetSwFilterSubPipe(true, layerIndexes[i]);
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureTypeWithEngineSingleLayer(featureSubPipe, caps, isolatedFeatureSelected));
    }

    if (outputPipeNeeded)
    {
        featureSubPipe = featurePipe.GetSwFilterSubPipe(false, 0);
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureTypeWithEngineSingleLayer(featureSubPipe, caps, false));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateFeatureTypeWithEngineSingleLayer(SwFilterSubPipe *featureSubPipe, VP_EXECUTE_CAPS& caps, bool isolatedFeatureSelected)
{
    bool isolatedFeatureFound = false;
    // Set feature types with engine
    for (auto filterID : m_featurePool)
    {
        auto feature = featureSubPipe->GetSwFilter(FeatureType(filterID));
        if (feature)
        {
            VP_EngineEntry *engineCaps = &(feature->GetFilterEngineCaps());

            if (isolatedFeatureSelected != engineCaps->isolated)
            {
                continue;
            }

            // if SFC enabled, Vebox is must as SFC need connect with Vebox
            if (caps.bSFC                           &&
                (engineCaps->forceEnableForSfc      ||
                engineCaps->bEnabled && (engineCaps->VeboxNeeded || engineCaps->SfcNeeded)))
            {
                if (engineCaps->forceEnableForSfc)
                {
                    engineCaps->bEnabled = 1;
                    engineCaps->SfcNeeded = 1;
                    caps.enableSFCLinearOutputByTileConvert |= engineCaps->enableSFCLinearOutputByTileConvert;
                }
                // Choose SFC as execution engine
                VP_PUBLIC_CHK_STATUS_RETURN(UpdateExeCaps(feature, caps, engineCaps->SfcNeeded ? EngineTypeVeboxSfc : EngineTypeVebox));
            }
            // Vebox only cases
            else if (caps.bVebox &&
                (engineCaps->bEnabled && engineCaps->VeboxNeeded || caps.bIECP && filterID == FeatureTypeCsc))
            {
                // If HDR filter exist, handle CSC previous to HDR in AddFiltersBasedOnCaps
                if (filterID == FeatureTypeCsc && IsHDRfilterExist(featureSubPipe))
                {
                    VP_PUBLIC_NORMALMESSAGE("HDR exist, handle CSC previous to HDR in AddFiltersBasedOnCaps");
                    continue;
                }

                if (!engineCaps->bEnabled && caps.bIECP && filterID == FeatureTypeCsc)
                {
                    engineCaps->bEnabled    = 1;
                    engineCaps->VeboxNeeded = 1;
                }

                VP_PUBLIC_CHK_STATUS_RETURN(UpdateExeCaps(feature, caps, EngineTypeVebox));
            }
            else if (caps.bRender && caps.bRenderHdr &&
                (engineCaps->forceEnableForHdrKernel ||
                engineCaps->bEnabled && engineCaps->hdrKernelSupported))
            {
                if (engineCaps->forceEnableForHdrKernel)
                {
                    engineCaps->bEnabled     = 1;
                    engineCaps->RenderNeeded = 1;
                    engineCaps->hdrKernelSupported = 1;
                }
                VP_PUBLIC_CHK_STATUS_RETURN(UpdateExeCaps(feature, caps, EngineTypeRender));
            }
            else if (caps.bRender                   &&
                (engineCaps->forceEnableForFc       ||
                engineCaps->bEnabled && engineCaps->RenderNeeded))
            {
                if (engineCaps->forceEnableForFc)
                {
                    engineCaps->bEnabled = 1;
                    engineCaps->RenderNeeded = 1;
                }
                // use render path to implement feature.
                VP_PUBLIC_CHK_STATUS_RETURN(UpdateExeCaps(feature, caps, EngineTypeRender));
            }

            if (engineCaps->isolated)
            {
                isolatedFeatureFound = true;
                break;
            }
        }
    }

    if (isolatedFeatureSelected && !isolatedFeatureFound)
    {
        VP_PUBLIC_ASSERTMESSAGE("Isolated feature is not found!");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe& featurePipe, bool isSingleSubPipe, uint32_t pipeIndex, VP_EXECUTE_CAPS& caps)
{
    layerIndexes.clear();
    if (isSingleSubPipe && !caps.bComposite && !caps.bRenderHdr)
    {
        layerIndexes.push_back(pipeIndex);
        return MOS_STATUS_SUCCESS;
    }

    if (caps.bRenderHdr)
    {
        auto it = m_RenderFeatureHandlers.find(FeatureTypeHdrOnRender);
        if (m_RenderFeatureHandlers.end() == it)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        PolicyRenderHdrHandler *hdrHandler = dynamic_cast<PolicyRenderHdrHandler *>(it->second);
        VP_PUBLIC_CHK_NULL_RETURN(hdrHandler);
        VP_PUBLIC_CHK_STATUS_RETURN(hdrHandler->LayerSelectForProcess(layerIndexes, featurePipe, caps));
        return MOS_STATUS_SUCCESS;
    }

    if (!caps.bComposite)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    auto it = m_RenderFeatureHandlers.find(FeatureTypeFcOnRender);
    if (m_RenderFeatureHandlers.end() == it)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    auto osInterface = m_vpInterface.GetHwInterface()->m_osInterface;

    for (uint32_t i = 0; i < featurePipe.GetSurfaceCount(true); ++i)
    {
        VP_SURFACE *input = featurePipe.GetSurface(true, i);
        VP_PUBLIC_CHK_NULL_RETURN(input);
        VP_PUBLIC_CHK_NULL_RETURN(input->osSurface);
        uint64_t gpuVa = osInterface->pfnGetResourceGfxAddress(osInterface, &input->osSurface->OsResource);

        VP_PUBLIC_NORMALMESSAGE("layer %d, gpuVa = 0x%llx", i, gpuVa);
    }

    VP_SURFACE *output = featurePipe.GetSurface(false, 0);
    VP_PUBLIC_CHK_NULL_RETURN(output);
    VP_PUBLIC_CHK_NULL_RETURN(output->osSurface);
    uint64_t gpuVa = osInterface->pfnGetResourceGfxAddress(osInterface, &output->osSurface->OsResource);
    VP_PUBLIC_NORMALMESSAGE("target, gpuVa = 0x%llx", gpuVa);

    PolicyFcWrapHandler *fcHandler = dynamic_cast<PolicyFcWrapHandler *>(it->second);
    VP_PUBLIC_CHK_NULL_RETURN(fcHandler);
    VP_PUBLIC_CHK_STATUS_RETURN(fcHandler->LayerSelectForProcess(layerIndexes, featurePipe, caps));

    if (layerIndexes.size() < featurePipe.GetSurfaceCount(true))
    {
        // Multi pass needed.
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface.GetResourceManager()->PrepareFcIntermediateSurface(featurePipe));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS AddInputSurfaceForSingleLayer(SwFilterPipe &featurePipe, uint32_t pipeIndex, SwFilterPipe &executedFilters, uint32_t &executePipeIndex, VP_EXECUTE_CAPS& caps)
{
    // Single layer add input surface
    if (caps.value)
    {
        // Move surfaces from subSwFilterPipe to executedFilters.
        VP_SURFACE *surfInput = featurePipe.GetSurface(true, pipeIndex);
        if (surfInput)
        {
            // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
            executePipeIndex = executedFilters.GetSurfaceCount(true);
            VP_PUBLIC_CHK_STATUS_RETURN(executedFilters.AddSurface(surfInput, true, executePipeIndex));
            VP_SURFACE *pastRefSurface = featurePipe.RemovePastSurface(pipeIndex);
            VP_SURFACE *futureRefSurface = featurePipe.RemoveFutureSurface(pipeIndex);
            executedFilters.SetPastSurface(executePipeIndex, pastRefSurface);
            executedFilters.SetFutureSurface(executePipeIndex, futureRefSurface);
            executedFilters.SetLinkedLayerIndex(executePipeIndex, pipeIndex);
        }
        else
        {
            VP_PUBLIC_ASSERTMESSAGE("No input for current pipe");
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::SetupExecuteFilter(SwFilterPipe& featurePipe, std::vector<int> &layerIndexes, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params.executedFilters);

    for (uint32_t i = 0; i < layerIndexes.size(); ++i)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AddInputSurfaceForSingleLayer(featurePipe, layerIndexes[i], *params.executedFilters, i, caps));
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeaturePipeSingleLayer(featurePipe, layerIndexes[i], *params.executedFilters, i, caps));
        VP_PUBLIC_CHK_STATUS_RETURN(AddFiltersBasedOnCaps(featurePipe, layerIndexes[i], caps, *params.executedFilters, i));
    }

    /* Place Holder: order pipe need to be insert in next step*/

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatureOutputPipe(layerIndexes, featurePipe, *params.executedFilters, caps));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateFeaturePipe(SwFilterPipe &featurePipe, uint32_t pipeIndex, SwFilterPipe &executedFilters, uint32_t executePipeIndex,
                                    bool isInputPipe, VP_EXECUTE_CAPS& caps)
{
    SwFilterSubPipe* featureSubPipe = featurePipe.GetSwFilterSubPipe(isInputPipe, pipeIndex);
    uint32_t featureSelected = 0;

    VP_PUBLIC_CHK_NULL_RETURN(featureSubPipe);
    // Move swfilter from feature sub pipe to params.executedFilters
    for (auto filterID : m_featurePool)
    {
        SwFilter *feature = (SwFilter*)featureSubPipe->GetSwFilter(FeatureType(filterID));
        if (feature)
        {
            VP_EngineEntry *engineCaps = &(feature->GetFilterEngineCaps());

            if (engineCaps->usedForNextPass)
            {
                continue;
            }

            if ((caps.bSFC || caps.bVebox) && engineCaps->bEnabled && IS_FEATURE_TYPE_ON_VEBOX_SFC(feature->GetFeatureType()) &&
                m_VeboxSfcFeatureHandlers.end() != m_VeboxSfcFeatureHandlers.find(feature->GetFeatureType()))
            {
                // Engine has been assigned to feature.
                auto it = m_VeboxSfcFeatureHandlers.find(feature->GetFeatureType());
                if (m_VeboxSfcFeatureHandlers.end() == it)
                {
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
                }
                PolicyFeatureHandler *handler = it->second;
                VP_PUBLIC_CHK_STATUS_RETURN(handler->UpdateFeaturePipe(caps, *feature, featurePipe, executedFilters, isInputPipe, executePipeIndex));
                if (!engineCaps->bEnabled)
                {
                    VP_PUBLIC_NORMALMESSAGE("filter is disable during UpdateFeaturePipe");
                }
                ++featureSelected;
            }
            else if (caps.bRender && engineCaps->bEnabled && IS_FEATURE_TYPE_ON_RENDER(feature->GetFeatureType()) &&
                    m_RenderFeatureHandlers.end() != m_RenderFeatureHandlers.find(feature->GetFeatureType()))
            {
                auto it = m_RenderFeatureHandlers.find(feature->GetFeatureType());
                if (m_RenderFeatureHandlers.end() == it)
                {
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
                }
                PolicyFeatureHandler *handler = it->second;

                VP_PUBLIC_CHK_STATUS_RETURN(handler->UpdateFeaturePipe(caps, *feature, featurePipe, executedFilters, isInputPipe, executePipeIndex));
                if (!engineCaps->bEnabled)
                {
                    VP_PUBLIC_NORMALMESSAGE("filter is disable during UpdateFeaturePipe");
                }
                ++featureSelected;
            }
            else
            {
                auto *handlers = (caps.bSFC || caps.bVebox) ? &m_VeboxSfcFeatureHandlers : &m_RenderFeatureHandlers;
                auto it = handlers->find(feature->GetFeatureType());
                if (handlers->end() != it)
                {
                    PolicyFeatureHandler *handler = it->second;
                    VP_PUBLIC_CHK_STATUS_RETURN(handler->UpdateUnusedFeature(caps, *feature, featurePipe, executedFilters, isInputPipe, executePipeIndex));
                }
            }

            if (!engineCaps->bEnabled && !engineCaps->usedForNextPass)
            {
                // Feature may be disabled during UpdateFeaturePipe, such as colorfill and alpha, which will
                // be combined into scaling in sfc.
                SwFilterFeatureHandler *handler = m_vpInterface.GetSwFilterHandler(feature->GetFeatureType());
                if (!handler)
                {
                    VP_PUBLIC_ASSERTMESSAGE("no Feature Handle, Return Pipe Init Error");
                    return MOS_STATUS_INVALID_HANDLE;
                }

                featurePipe.RemoveSwFilter(feature);
                handler->Destory(feature);
                VP_PUBLIC_NORMALMESSAGE("filter is disable during UpdateFeaturePipe");
            }
        }
    }

    if (!isInputPipe && featureSelected && !featureSubPipe->IsEmpty() && featurePipe.IsAllInputPipeSurfaceFeatureEmpty())
    {
        VP_PUBLIC_ASSERTMESSAGE("Not all output features being selected!");
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateFeaturePipeSingleLayer(SwFilterPipe &featurePipe, uint32_t pipeIndex, SwFilterPipe &executedFilters, uint32_t executePipeIndex, VP_EXECUTE_CAPS& caps)
{
    VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeaturePipe(featurePipe, pipeIndex, executedFilters, executePipeIndex, true, caps));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateFeatureOutputPipe(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, SwFilterPipe &executedFilters, VP_EXECUTE_CAPS& caps)
{
    if (!caps.bOutputPipeFeatureInuse)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (!featurePipe.IsAllInputPipeSurfaceFeatureEmpty(layerIndexes))
    {
        VP_PUBLIC_ASSERTMESSAGE("bOutputPipeFeatureInuse being set but input pipe is not empty.");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeaturePipe(featurePipe, 0, executedFilters, 0, false, caps));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::SetupFilterResource(SwFilterPipe& featurePipe, std::vector<int> &layerIndexes, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_SURFACE* surfInput  = nullptr;
    VP_SURFACE* surfOutput = nullptr;
    uint32_t i = 0;

    if (featurePipe.IsEmpty())
    {
        // If all subpipes are empty, which means no swfilter exists in featurePipe, just move output surface from featurePipe to executedFilters.
        // In such case, processing complete.
        // Update the input feature surfaces
        surfOutput = featurePipe.RemoveSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
        // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
        VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfOutput, false, 0));
        VP_PUBLIC_NORMALMESSAGE("Output surface in use, since no filters left in featurePipe.");
    }
    else if (RenderTargetTypeParameter == featurePipe.GetRenderTargetType() ||
             RenderTargetTypeParameter == params.executedFilters->GetRenderTargetType())
    {
        surfOutput = featurePipe.GetSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
        VP_SURFACE *surfOutput2 = m_vpInterface.GetAllocator().AllocateVpSurface(*surfOutput);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput2);
        VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfOutput2, false, 0));
        VP_PUBLIC_NORMALMESSAGE("Output surface in use, since only parameter filters in featurePipe.");
    }
    else if (caps.bOutputPipeFeatureInuse)
    {
        // Use intermedia surfaces for multi layer case, or color fill during composition may not
        // act correctly.
        VP_PUBLIC_NORMALMESSAGE("Intermedia surface in use with 1 == bOutputPipeFeatureInuse.");
    }
    else if (IsSecureResourceNeeded(caps))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateSecureExecuteResource(featurePipe, caps, params));
    }
    else
    {
        // If not assign output surface, intermedia surface will be assigned in AssignExecuteResource.
    }

    VP_PUBLIC_CHK_STATUS_RETURN(AssignExecuteResource(caps, params));

    SwFilterSubPipe *subPipe = nullptr;

    if (1 == layerIndexes.size())
    {
        subPipe = featurePipe.GetSwFilterSubPipe(true, layerIndexes[0]);
    }

    if (featurePipe.IsEmpty())
    {
        // Update the input feature surfaces
        for (i = 0; i < layerIndexes.size(); ++i)
        {
            surfInput = featurePipe.RemoveSurface(true, layerIndexes[i]);
        }
    }
    else if (subPipe && RenderTargetTypeParameter == subPipe->GetRenderTargetType() ||
             RenderTargetTypeParameter == params.executedFilters->GetRenderTargetType())
    {
        surfInput = featurePipe.GetSurface(true, layerIndexes[0]);
        VP_PUBLIC_CHK_NULL_RETURN(surfInput);
        VP_SURFACE* input = m_vpInterface.GetAllocator().AllocateVpSurface(*surfInput);
        VP_PUBLIC_CHK_NULL_RETURN(input);
        input->SurfType = SURF_IN_PRIMARY;
        featurePipe.ReplaceSurface(input, true, layerIndexes[0]);
    }
    else if (IsSecureResourceNeeded(caps))
    {
        VP_PUBLIC_NORMALMESSAGE("Secure Process Enabled, no need further process");
    }
    else
    {
        auto osInterface = m_vpInterface.GetHwInterface()->m_osInterface;
        auto outputSurfaceExePipe = params.executedFilters->GetSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(outputSurfaceExePipe);
        auto outputSurface = featurePipe.GetSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(outputSurface);
        bool outputSurfaceInuse =
            outputSurfaceExePipe->GetAllocationHandle(osInterface) == outputSurface->GetAllocationHandle(osInterface);

        if (outputSurfaceInuse)
        {
            // Update the input feature surfaces
            for (i = 0; i < layerIndexes.size(); ++i)
            {
                surfInput = featurePipe.RemoveSurface(true, layerIndexes[i]);
            }
        }
        else
        {
            // multi-pass with intermedia surface case.
            auto intermediaSurface = outputSurfaceExePipe;
            VP_SURFACE *input =  m_vpInterface.GetAllocator().AllocateVpSurface(*intermediaSurface);
            VP_PUBLIC_CHK_NULL_RETURN(input);

            // For FC, also reuse first pipe for the composition layer in previous steps.
            auto originInput = featurePipe.GetSurface(true, layerIndexes[0]);
            if (originInput == nullptr)
            {
                m_vpInterface.GetAllocator().DestroyVpSurface(input);
                VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
            }

            input->SurfType = originInput->SurfType;
            featurePipe.ReplaceSurface(input, true, layerIndexes[0]);

            // Update other input surfaces
            for (i = 1; i < layerIndexes.size(); ++i)
            {
                surfInput = featurePipe.RemoveSurface(true, layerIndexes[i]);
            }

            auto swFilterSubPipe = featurePipe.GetSwFilterSubPipe(true, layerIndexes[0]);
            VP_PUBLIC_CHK_NULL_RETURN(swFilterSubPipe);

            VP_SURFACE *output = featurePipe.GetSurface(false, 0);
            VP_PUBLIC_CHK_NULL_RETURN(output);

            // swFilters will be updated later in Policy::BuildExecuteFilter.
            VP_PUBLIC_CHK_STATUS_RETURN(AddCommonFilters(*swFilterSubPipe, input, output));
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::AddCommonFilters(SwFilterSubPipe &swFilterSubPipe, VP_SURFACE *input, VP_SURFACE *output)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(input);
    VP_PUBLIC_CHK_NULL_RETURN(output);

    MOS_STATUS      status        = MOS_STATUS_SUCCESS;
    FeatureType featureList[] = { FeatureTypeScaling };
    int32_t featureCount = sizeof(featureList) / sizeof(featureList[0]);
    VP_EXECUTE_CAPS caps = {};

    for (int32_t i = 0; i < featureCount; ++i)
    {
        FeatureType featureType = featureList[i];

        SwFilter *swFilter = swFilterSubPipe.GetSwFilter(featureType);
        if (nullptr != swFilter)
        {
            continue;
        }

        VP_PUBLIC_NORMALMESSAGE("Feature %d is added.", FeatureTypeScaling);

        caps.value = 0;

        SwFilterFeatureHandler *handler = m_vpInterface.GetSwFilterHandler(featureType);
        VP_PUBLIC_CHK_NULL_RETURN(handler);
        swFilter = handler->CreateSwFilter();
        VP_PUBLIC_CHK_NULL_RETURN(swFilter);

        status = swFilter->Configure(input, output, caps);
        if (MOS_FAILED(status))
        {
            handler->Destory(swFilter);
            VP_PUBLIC_CHK_STATUS_RETURN(status);
        }
        status = swFilterSubPipe.AddSwFilterUnordered(swFilter);
        if (MOS_FAILED(status))
        {
            handler->Destory(swFilter);
            VP_PUBLIC_CHK_STATUS_RETURN(status);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateExeCaps(SwFilter* feature, VP_EXECUTE_CAPS& caps, EngineType Type)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    FeatureType featureType = feature->GetFeatureType();

    if (Type == EngineTypeVeboxSfc)
    {
        switch (featureType)
        {
        case FeatureTypeCsc:
            caps.bSfcCsc = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Csc, Sfc)));
            break;
        case FeatureTypeScaling:
            caps.bSfcScaling = 1;
            if (feature->GetFilterEngineCaps().sfc2PassScalingNeededX || feature->GetFilterEngineCaps().sfc2PassScalingNeededY)
            {
                caps.b1stPassOfSfc2PassScaling = true;
            }
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Scaling, Sfc)));
            break;
        case FeatureTypeRotMir:
            caps.bSfcRotMir = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(RotMir, Sfc)));
            break;
        case FeatureTypeColorFill:
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(ColorFill, Sfc)));
            break;
        case FeatureTypeAlpha:
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Alpha, Sfc)));
            break;
        default:
            break;
        }
    }

    if (Type == EngineTypeVebox)
    {
        switch (featureType)
        {
        case FeatureTypeDn:
            caps.bDN = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Dn, Vebox)));
            break;
        case FeatureTypeSte:
            caps.bSTE = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Ste, Vebox)));
            break;
        case FeatureTypeDi:
            caps.bDI = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Di, Vebox)));
            break;
        case FeatureTypeTcc:
            caps.bTCC = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Tcc, Vebox)));
            break;
        case FeatureTypeProcamp:
            if (caps.bForceProcampToRender)
            {
                caps.bProcamp = 0;
                break;
            }
            caps.bProcamp = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Procamp, Vebox)));
            break;
        case FeatureTypeCsc:
            caps.bBeCSC = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Csc, Vebox)));
            break;
        case FeatureTypeHdr:
            caps.bHDR3DLUT = 1;
            caps.b3DlutOutput |= 1;
            if (feature->GetFilterEngineCaps().isHdr33LutSizeEnabled)
            {
                caps.bHdr33lutsize = 1;
            }
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Hdr, Vebox)));
            break;
        case FeatureTypeCgc:
            caps.bCGC = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Cgc, Vebox)));
            VP_PUBLIC_CHK_STATUS_RETURN(UpdateCGCMode(feature, caps, Type));
            break;
        default:
            break;
        }
    }

    if (Type == EngineTypeRender)
    {
        switch (featureType)
        {
        case FeatureTypeCsc:
            caps.bComposite = caps.bRenderHdr ? 0 : 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Csc, Render)));
            break;
        case FeatureTypeScaling:
            caps.bComposite = caps.bRenderHdr ? 0 : 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Scaling, Render)));
            break;
        case FeatureTypeRotMir:
            caps.bComposite = caps.bRenderHdr ? 0 : 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(RotMir, Render)));
            break;
        case FeatureTypeProcamp:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Procamp, Render)));
            break;
        case FeatureTypeDi:
            caps.bDI          = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Di, Render)));
            break;
        case FeatureTypeLumakey:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Lumakey, Render)));
            break;
        case FeatureTypeBlending:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Blending, Render)));
            break;
        case FeatureTypeColorFill:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(ColorFill, Render)));
            break;
        case FeatureTypeAlpha:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Alpha, Render)));
            break;
        case FeatureTypeHdr:
            if (feature->GetFilterEngineCaps().isolated)
            {
                caps.b3DLutCalc = 1;
                if (feature->GetFilterEngineCaps().isHdr33LutSizeEnabled)
                {
                    caps.bHdr33lutsize = 1;
                }
                feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Hdr3DLutCal, Render)));
            }
            else
            {
                caps.bHdr = 1;
                feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Hdr, Render)));
            }
            break;
        case FeatureTypeDn:
            if (feature->GetFilterEngineCaps().isolated)
            {
                caps.bHVSCalc = 1;
                feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(DnHVSCal, Render)));
            }
            else
            {
                VP_PUBLIC_ASSERTMESSAGE("DN HVS Kernel has not been enabled in APO path");
            }
            break;
        default:
            break;
        }

        if (caps.bComposite && caps.bRenderHdr)
        {
            VP_PUBLIC_ASSERTMESSAGE("FC and Render HDR should not be selected at same time.");
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateCGCMode(SwFilter* feature, VP_EXECUTE_CAPS& caps, EngineType Type)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(feature);
    SwFilterCgc  *cgcFilter = dynamic_cast<SwFilterCgc*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(cgcFilter);

    caps.bBt2020ToRGB = (caps.bCGC && cgcFilter->IsBt2020ToRGBEnabled()) ? 1 : 0;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::AssignExecuteResource(VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(params.executedFilters);
    VP_PUBLIC_CHK_NULL_RETURN(m_vpInterface.GetResourceManager());
    VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface.GetResourceManager()->AssignExecuteResource(m_featurePool, caps, *params.executedFilters));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildVeboxSecureFilters(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::ReleaseHwFilterParam(HW_FILTER_PARAMS &params)
{
    VP_FUNC_CALL();

    if (EngineTypeInvalid == params.Type || params.Params.empty())
    {
        params.Type = EngineTypeInvalid;
        while (!params.Params.empty())
        {
            HwFilterParameter *p = params.Params.back();
            params.Params.pop_back();
            MOS_Delete(p);
        }

        m_vpInterface.GetSwFilterPipeFactory().Destory(params.executedFilters);

        return MOS_STATUS_SUCCESS;
    }

    std::map<FeatureType, PolicyFeatureHandler*> &featureHandler = 
            (EngineTypeVebox == params.Type || EngineTypeVeboxSfc == params.Type) ? m_VeboxSfcFeatureHandlers : m_RenderFeatureHandlers;

    params.Type = EngineTypeInvalid;
    while (!params.Params.empty())
    {
        HwFilterParameter *p = params.Params.back();
        params.Params.pop_back();
        if (p)
        {
            auto it = featureHandler.find(p->GetFeatureType());
            if (featureHandler.end() == it)
            {
                MOS_Delete(p);
            }
            else
            {
                it->second->ReleaseHwFeatureParameter(p);
            }
        }
    }

    m_vpInterface.GetSwFilterPipeFactory().Destory(params.executedFilters);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::AddFiltersBasedOnCaps(
    SwFilterPipe& featurePipe,
    uint32_t pipeIndex,
    VP_EXECUTE_CAPS& caps,
    SwFilterPipe& executedFilters,
    uint32_t executedPipeIndex)
{
    VP_FUNC_CALL();

    // Create and Add CSC filter for VEBOX IECP chromasiting config
    // HDR State holder: To keep same as Legacy path -- for VE 3DLut HDR, enable VE chroma up sampling when ONLY VE output.
    if (!caps.bBeCSC && ((caps.bSFC && (caps.bIECP || caps.bDI)) || (!caps.bSFC && (caps.bIECP || caps.b3DlutOutput || caps.bBt2020ToRGB))))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AddNewFilterOnVebox(featurePipe, pipeIndex, caps, executedFilters, executedPipeIndex, FeatureTypeCsc));
    }
    else
    {
        if (caps.bBeCSC && caps.bHDR3DLUT)
        {
            // bBeCSC won't be set in GetCSCExecutionCaps for HDR case
            VP_PUBLIC_ASSERTMESSAGE("bBeCSC shouldn't be set in GetCSCExecutionCaps for HDR case");
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::AddNewFilterOnVebox(
    SwFilterPipe& featurePipe,
    uint32_t pipeIndex,
    VP_EXECUTE_CAPS& caps,
    SwFilterPipe& executedFilters,
    uint32_t executedPipeIndex,
    FeatureType featureType)
{
    VP_FUNC_CALL();

    MOS_STATUS  status      = MOS_STATUS_SUCCESS;
    PVP_SURFACE pSurfInput = featurePipe.GetSurface(true, pipeIndex);
    PVP_SURFACE pSurfOutput = featurePipe.GetSurface(false, 0);
    VP_PUBLIC_CHK_NULL_RETURN(pSurfInput);
    VP_PUBLIC_CHK_NULL_RETURN(pSurfOutput);

    auto handler = m_vpInterface.GetSwFilterHandler(featureType);

    if (!handler)
    {
        VP_PUBLIC_ASSERTMESSAGE("no Feature Handle, Return Pipe Init Error");
        return MOS_STATUS_INVALID_HANDLE;
    }

    SwFilter* swfilter = handler->CreateSwFilter();
    VP_PUBLIC_CHK_NULL_RETURN(swfilter);

    if (featureType == FeatureTypeCsc)
    {
        SwFilterCsc *csc = (SwFilterCsc *)swfilter;
        FeatureParamCsc cscParams = csc->GetSwFilterParams();
        status                    = GetCscParamsOnCaps(pSurfInput, pSurfOutput, caps, cscParams);
        if (MOS_FAILED(status))
        {
            handler->Destory(swfilter);
            VP_PUBLIC_CHK_STATUS_RETURN(status);
        }
        else
        {
            status = csc->Configure(cscParams);
        }
    }
    else if (featureType == FeatureTypeDn)
    {
        SwFilterDenoise *dn        = (SwFilterDenoise *)swfilter;
        FeatureParamDenoise dnParams = dn->GetSwFilterParams();
        status                        = GetDnParamsOnCaps(pSurfInput, pSurfOutput, caps, dnParams);
        if (MOS_FAILED(status))
        {
            handler->Destory(swfilter);
            VP_PUBLIC_CHK_STATUS_RETURN(status);
        }
        else
        {
            status = dn->Configure(dnParams);
        }
    }
    else
    {
        status = swfilter->Configure(pSurfInput, pSurfOutput, caps);
    }

    if (MOS_FAILED(status))
    {
        handler->Destory(swfilter);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateExeCaps(swfilter, caps, EngineTypeVebox));

    status = executedFilters.AddSwFilterUnordered(swfilter, true, executedPipeIndex);
    VP_PUBLIC_CHK_STATUS_RETURN(status);

    return status;
}

MOS_STATUS GetVeboxOutputParams(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, MOS_TILE_TYPE inputTileType, MOS_FORMAT outputFormat, MOS_FORMAT &veboxOutputFormat, MOS_TILE_TYPE &veboxOutputTileType, VPHAL_CSPACE colorSpaceOutput);

MOS_STATUS Policy::GetCscParamsOnCaps(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS &caps, FeatureParamCsc &cscParams)
{
    if (caps.bHDR3DLUT)
    {
        cscParams.input.colorSpace   = surfInput->ColorSpace;
        cscParams.formatInput        = surfInput->osSurface->Format;
        cscParams.formatforCUS       = Format_None;
        cscParams.input.chromaSiting = surfInput->ChromaSiting;

        // CSC before HDR converts BT2020 P010 to ARGB10
        cscParams.output.colorSpace  = CSpace_BT2020_RGB;
        cscParams.formatOutput       = Format_B10G10R10A2;
        cscParams.output.chromaSiting = surfOutput->ChromaSiting;

        cscParams.pAlphaParams = nullptr;
        cscParams.pIEFParams   = nullptr;

        return MOS_STATUS_SUCCESS;
    }
    else if (caps.bSFC)
    {
        MOS_FORMAT    veboxOutputFormat   = surfInput->osSurface->Format;
        MOS_TILE_TYPE veboxOutputTileType = surfInput->osSurface->TileType;

        GetVeboxOutputParams(caps, surfInput->osSurface->Format, surfInput->osSurface->TileType, surfOutput->osSurface->Format, veboxOutputFormat, veboxOutputTileType, surfOutput->ColorSpace);
        cscParams.input.colorSpace = surfInput->ColorSpace;
        cscParams.output.colorSpace = surfInput->ColorSpace;

        cscParams.formatInput         = surfInput->osSurface->Format;
        cscParams.formatforCUS        = Format_None;
        cscParams.formatOutput        = veboxOutputFormat;
        cscParams.input.chromaSiting  = surfInput->ChromaSiting;
        cscParams.output.chromaSiting = surfOutput->ChromaSiting;

        cscParams.pAlphaParams = nullptr;
        cscParams.pIEFParams   = nullptr;

        return MOS_STATUS_SUCCESS;
    }
    else if (caps.bBt2020ToRGB || caps.bIECP)
    {
        // For BeCsc filter added in policy based on caps, it's for chroma-sitting and
        // SetVeboxIecpStateBecsc to use.
        // For BT2020 to RGB case, the VeboxInterface_BT2020YUVToRGB will be used for BeCsc setting,
        // which will overwritten the one by SetVeboxIecpStateBecsc.
        // So in GetCscParamsOnCaps for BT2020ToRGB case,
        // we just need to use same format and colorspace for both input and output,
        // just use the input format to ensure chroma-sitting parameters being correct.
        cscParams.input.colorSpace   = surfInput->ColorSpace;
        cscParams.formatInput        = surfInput->osSurface->Format;
        cscParams.formatforCUS       = Format_None;
        cscParams.input.chromaSiting = surfInput->ChromaSiting;

        cscParams.output.colorSpace   = surfInput->ColorSpace;
        cscParams.formatOutput        = surfInput->osSurface->Format;
        cscParams.output.chromaSiting = surfOutput->ChromaSiting;

        cscParams.pAlphaParams = nullptr;
        cscParams.pIEFParams   = nullptr;

        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_UNIMPLEMENTED;
}

MOS_STATUS Policy::GetDnParamsOnCaps(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS &caps, FeatureParamDenoise &dnParams)
{
    dnParams.formatInput     = surfInput->osSurface->Format;
    dnParams.heightInput     = surfInput->osSurface->dwHeight;
    dnParams.formatOutput    = Format_NV12;
    dnParams.sampleTypeInput = SAMPLE_PROGRESSIVE;

    dnParams.denoiseParams.bEnableLuma       = true;
    dnParams.denoiseParams.bEnableChroma     = true;
    dnParams.denoiseParams.bEnableHVSDenoise = false;
    dnParams.denoiseParams.bAutoDetect       = false;
    dnParams.denoiseParams.fDenoiseFactor    = 64.0f;

#if !EMUL
    GMM_RESOURCE_INFO *pSrcGmmResInfo    = surfInput->osSurface->OsResource.pGmmResInfo;
    GMM_RESOURCE_INFO *pTargetGmmResInfo = surfOutput->osSurface->OsResource.pGmmResInfo;
    VP_PUBLIC_CHK_NULL_RETURN(pSrcGmmResInfo);
    VP_PUBLIC_CHK_NULL_RETURN(pTargetGmmResInfo);

    bool inputProtected  = pSrcGmmResInfo->GetSetCpSurfTag(0, 0);
    bool outputProtected = pTargetGmmResInfo->GetSetCpSurfTag(0, 0);

    if (inputProtected || outputProtected ||
        (m_vpInterface.GetHwInterface()->m_osInterface->osCpInterface &&
            m_vpInterface.GetHwInterface()->m_osInterface->osCpInterface->IsHMEnabled()))
    {
        dnParams.secureDnNeeded = true;
    }
#endif

    return MOS_STATUS_SUCCESS;
}

void Policy::PrintFeatureExecutionCaps(const char *name, VP_EngineEntry &engineCaps)
{
    VP_PUBLIC_NORMALMESSAGE("%s, value 0x%x (bEnabled %d, VeboxNeeded %d, SfcNeeded %d, RenderNeeded %d, fcSupported %d, isolated %d, veboxNotSupported %d, sfcNotSupported %d)",
        name, engineCaps.value, engineCaps.bEnabled, engineCaps.VeboxNeeded, engineCaps.SfcNeeded,
        engineCaps.RenderNeeded, engineCaps.fcSupported, engineCaps.isolated, engineCaps.veboxNotSupported, engineCaps.sfcNotSupported);
}

};
