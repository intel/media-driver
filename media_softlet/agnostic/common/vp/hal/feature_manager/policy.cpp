/*
* Copyright (c) 2019-2021, Intel Corporation
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

using namespace vp;

/****************************************************************************************************/
/*                                      Policy                                                      */
/****************************************************************************************************/

Policy::Policy(VpInterface &vpInterface) : m_vpInterface(vpInterface)
{
    MOS_USER_FEATURE_VALUE_DATA UserFeatureData;
    // Read user feature key to get the Composition Bypass mode
    MOS_ZeroMemory(&UserFeatureData, sizeof(UserFeatureData));
    UserFeatureData.i32DataFlag = MOS_USER_FEATURE_VALUE_DATA_FLAG_CUSTOM_DEFAULT_VALUE_TYPE;

    // Vebox Comp Bypass is on by default
    UserFeatureData.u32Data = VP_COMP_BYPASS_ENABLED;

    MOS_USER_FEATURE_INVALID_KEY_ASSERT(MOS_UserFeature_ReadValue_ID(
        nullptr,
        __VPHAL_BYPASS_COMPOSITION_ID,
        &UserFeatureData,
        m_vpInterface.GetHwInterface() && m_vpInterface.GetHwInterface()->m_osInterface ? m_vpInterface.GetHwInterface()->m_osInterface->pOsContext : nullptr));
    m_bypassCompMode = UserFeatureData.u32Data;
}

Policy::~Policy()
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

}

MOS_STATUS Policy::Initialize()
{
    VP_FUNC_CALL();

    VpPlatformInterface *vpPlatformInterface = (VpPlatformInterface *)m_vpInterface.GetHwInterface()->m_vpPlatformInterface;
    VP_PUBLIC_CHK_NULL_RETURN(vpPlatformInterface);
    VP_PUBLIC_CHK_STATUS_RETURN(vpPlatformInterface->InitVpHwCaps(m_hwCaps));
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

    p = MOS_New(PolicyDiHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeDiOnVebox, p));

    p = MOS_New(PolicySfcColorFillHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeColorFillOnSfc, p));

    p = MOS_New(PolicySfcAlphaHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeAlphaOnSfc, p));

    p = MOS_New(PolicyDiHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_RenderFeatureHandlers.insert(std::make_pair(FeatureTypeDiFmdOnRender, p));

    // Next step to add a table to trace all SW features based on platforms
    m_featurePool.clear();
    m_featurePool.push_back(FeatureTypeCsc);
    m_featurePool.push_back(FeatureTypeScaling);
    m_featurePool.push_back(FeatureTypeRotMir);
    m_featurePool.push_back(FeatureTypeDn);
    m_featurePool.push_back(FeatureTypeSte);
    m_featurePool.push_back(FeatureTypeTcc);
    m_featurePool.push_back(FeatureTypeProcamp);
    m_featurePool.push_back(FeatureTypeHdr);
    m_featurePool.push_back(FeatureTypeDi);
    m_featurePool.push_back(FeatureTypeColorFill);
    m_featurePool.push_back(FeatureTypeAlpha);

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

    MOS_STATUS status = GetHwFilterParam(subSwFilterPipe, param);

    if (MOS_FAILED(status))
    {
        VP_PUBLIC_ASSERTMESSAGE("Create HW Filter Failed, Return Error");
        return status;
    }

    pFilter = m_vpInterface.GetHwFilterFactory().Create(param);

    ReleaseHwFilterParam(param);

    if (!pFilter)
    {
        VP_PUBLIC_ASSERTMESSAGE("Create HW Filter Failed, Return Error");
        return MOS_STATUS_UNIMPLEMENTED;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetHwFilterParam(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    MOS_STATUS status;

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
            return status;
        }
    }

    status = GetExecuteCaps(subSwFilterPipe, params);

    if (status != MOS_STATUS_SUCCESS)
    {
        m_vpInterface.GetSwFilterPipeFactory().Destory(params.executedFilters);
        VP_PUBLIC_ASSERTMESSAGE("Create Executed Filter Failed, Return Error");
        return status;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecuteCaps(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_EXECUTE_CAPS  caps = {};
    SwFilterSubPipe* pipe = nullptr;
    uint32_t index = 0;

    VP_PUBLIC_NORMALMESSAGE("Only Support primary layer for advanced processing");

    uint32_t inputSurfCount     = subSwFilterPipe.GetSurfaceCount(true);
    uint32_t outputSurfCount    = subSwFilterPipe.GetSurfaceCount(false);

    for (index = 0; index < inputSurfCount; ++index)
    {
        pipe = subSwFilterPipe.GetSwFilterSubPipe(true, index);
        if (pipe)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(BuildExecutionEngines(*pipe));
        }
    }

    for (index = 0; index < outputSurfCount; ++index)
    {
        pipe = subSwFilterPipe.GetSwFilterSubPipe(false, index);
        if (pipe)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(BuildExecutionEngines(*pipe));
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(BuildFilters(subSwFilterPipe, params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecutionCapsForSingleFeature(FeatureType featureType, SwFilterSubPipe& swFilterPipe)
{
    VP_FUNC_CALL();

    SwFilter* feature = swFilterPipe.GetSwFilter(featureType);
    SwFilter* diFilter = nullptr;

    if (!feature)
    {
        VP_PUBLIC_NORMALMESSAGE("Feature %d is not enabled in current pipe", featureType);
        return MOS_STATUS_SUCCESS;
    }
    else
    {
        VP_PUBLIC_NORMALMESSAGE("Feature %d is enabled in current pipe", featureType);
    }

    // Always clean engine caps before Query. For next step, need to add new caps variable in swFilter
    // for forcing engine case.
    feature->GetFilterEngineCaps().value = 0;

    switch (featureType)
    {
    case FeatureTypeCsc:
        diFilter = swFilterPipe.GetSwFilter(FeatureTypeDi);
        if (diFilter)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCapsDi(feature));
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCaps(feature));
        }
        break;
    case FeatureTypeScaling:
        VP_PUBLIC_CHK_STATUS_RETURN(GetScalingExecutionCaps(feature));
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
        VP_PUBLIC_CHK_STATUS_RETURN(GetDeinterlaceExecutionCaps(feature));
        break;
    case FeatureTypeColorFill:
        VP_PUBLIC_CHK_STATUS_RETURN(GetColorFillExecutionCaps(feature));
        break;
    case FeatureTypeAlpha:
        VP_PUBLIC_CHK_STATUS_RETURN(GetAlphaExecutionCaps(feature));
        break;
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(GetExecutionCaps(feature));
        VP_PUBLIC_ASSERTMESSAGE("Feature is not supported by driver, bypass it");
        break;
    }

    if (feature->GetFilterEngineCaps().value == 0)
    {
        // Return success after all feature enabled and fully switched to APO path.
        VP_PUBLIC_NORMALMESSAGE("No engine being assigned for feature %d. Will bypass it.", featureType);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecutionEngines(SwFilterSubPipe& swFilterPipe)
{
    VP_FUNC_CALL();

    SwFilter* feature = nullptr;
    for (auto filterID : m_featurePool)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(GetExecutionCapsForSingleFeature(filterID, swFilterPipe));
        VP_PUBLIC_CHK_STATUS_RETURN(FilterFeatureCombination(&swFilterPipe));
    }
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
    //HDR CSC processing
    if (!hdrParams || hdrParams->hdrMode == VPHAL_HDR_MODE_NONE)
    {
        VP_PUBLIC_ASSERTMESSAGE("HDR Mode is NONE");
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    MOS_FORMAT   hdrFormat  = Format_Any;
    VPHAL_CSPACE hdrCSpace  = CSpace_Any;
    hdrCSpace               = IS_COLOR_SPACE_BT2020(cscParams->output.colorSpace) ? CSpace_BT2020_RGB : CSpace_sRGB;
    hdrFormat               = IS_COLOR_SPACE_BT2020(cscParams->output.colorSpace) ? Format_R10G10B10A2 : Format_A8R8G8B8;
    if (m_hwCaps.m_sfcHwEntry[hdrFormat].inputSupported &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
        m_hwCaps.m_sfcHwEntry[hdrFormat].cscSupported)
    {

        if (hdrFormat == cscParams->formatOutput && hdrCSpace == cscParams->output.colorSpace)
        {
            VP_PUBLIC_NORMALMESSAGE("Skip CSC for HDR case.");
            cscEngine->forceEnableForSfc = 1;
        }
        else
        {
            cscEngine->bEnabled     = 1;
            cscEngine->SfcNeeded    = 1;
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

    VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCaps(feature));

    VP_EngineEntry *cscEngine = &csc->GetFilterEngineCaps();
    VP_PUBLIC_CHK_NULL_RETURN(cscEngine);
    if ((cscEngine->bEnabled && (cscEngine->SfcNeeded || cscEngine->VeboxNeeded)) ||
        !cscEngine->bEnabled && cscEngine->forceEnableForSfc)
    {
        cscEngine->bEnabled     = 1;
        cscEngine->SfcNeeded    = 1;
        cscEngine->VeboxNeeded  = 0;
        cscEngine->RenderNeeded = 0;
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetCSCExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterCsc* csc = (SwFilterCsc*)feature;

    FeatureParamCsc *cscParams = &csc->GetSwFilterParams();

    MOS_FORMAT midFormat = Format_Any;

    VP_EngineEntry *cscEngine = &csc->GetFilterEngineCaps();

    if (cscEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("CSC Feature Already been processed, Skip further process");
        // Clean usedForNextPass flag.
        if (cscEngine->usedForNextPass)
        {
            cscEngine->usedForNextPass = false;
        }
        return MOS_STATUS_SUCCESS;
    }

    if (m_bypassCompMode                != VP_COMP_BYPASS_DISABLED          &&
        cscParams->formatInput          == cscParams->formatOutput          &&
        cscParams->input.colorSpace     == cscParams->output.colorSpace     &&
        cscParams->input.chromaSiting   == cscParams->output.chromaSiting   &&
        nullptr                         == cscParams->pIEFParams)
    {
        // for non-csc cases, all engine supported
        cscEngine->bEnabled             = 0;
        cscEngine->SfcNeeded            = 0;
        cscEngine->VeboxNeeded          = 0;
        cscEngine->RenderNeeded         = 0;
        cscEngine->forceEnableForSfc    = 1;
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
            midFormat = Format_A8R8G8B8; // Vebox Gamut compression is needed, Vebox output is ARGB as SFC input
        }
        else
        {
            midFormat = Format_Any;
        }
    }

    if (midFormat != Format_Any)
    {
        if (m_hwCaps.m_sfcHwEntry[midFormat].cscSupported &&
            m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
            m_hwCaps.m_sfcHwEntry[midFormat].inputSupported)
        {
            // Vebox GC + SFC CSC to handle legacy 2 pass CSC case.
            cscEngine->bEnabled    = 1;
            cscEngine->SfcNeeded   = 1;
            cscEngine->VeboxNeeded = 0;

            return MOS_STATUS_SUCCESS;
        }
        else
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
    }

    cscEngine->bEnabled     = 1;
    cscEngine->RenderNeeded = 1;
    cscEngine->fcSupported  = 1;

    // SFC CSC enabling check
    if (m_hwCaps.m_sfcHwEntry[cscParams->formatInput].inputSupported   &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatInput].cscSupported)
    {
        cscEngine->SfcNeeded = 1;
    }

    // if vebox bypass composition mode disabled, then such mega feature like CSC will not take effect in Vebox.
    // then CSC will must be assign to SFC/Render path for execution.
    if (m_bypassCompMode != VP_COMP_BYPASS_DISABLED                              &&
        !cscParams->pIEFParams                                                   &&
       (!cscParams->pAlphaParams                                                 ||
         cscParams->pAlphaParams->AlphaMode != VPHAL_ALPHA_FILL_MODE_BACKGROUND) &&
        m_hwCaps.m_veboxHwEntry[cscParams->formatInput].inputSupported                    &&
        m_hwCaps.m_veboxHwEntry[cscParams->formatOutput].outputSupported                  &&
        m_hwCaps.m_veboxHwEntry[cscParams->formatInput].iecp                              &&
        m_hwCaps.m_veboxHwEntry[cscParams->formatInput].backEndCscSupported)
    {
        cscEngine->VeboxNeeded = 1;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetScalingExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();

    uint32_t dwSurfaceWidth = 0, dwSurfaceHeight = 0;
    uint32_t dwOutputSurfaceWidth = 0, dwOutputSurfaceHeight = 0;
    uint32_t dwSfcMinWidth = 0, dwSfcMaxWidth = 0;
    uint32_t dwSfcMinHeight = 0, dwSfcMaxHeight = 0;
    uint32_t dwDstMinHeight = 0;
    float    fScaleMin = 0, fScaleMax = 0;
    float    fScaleMin2Pass = 0, fScaleMax2Pass = 0;
    float    fScaleX = 0, fScaleY = 0;

    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterScaling* scaling = (SwFilterScaling*)feature;

    FeatureParamScaling *scalingParams = &scaling->GetSwFilterParams();

    VP_EngineEntry *scalingEngine = &scaling->GetFilterEngineCaps();

    if (scalingEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");
        // Clean usedForNextPass flag.
        if (scalingEngine->usedForNextPass)
        {
            scalingEngine->usedForNextPass = false;
        }
        return MOS_STATUS_SUCCESS;
    }

    dwSfcMinWidth  = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].minResolution;
    dwSfcMaxWidth  = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].maxResolution;
    dwSfcMinHeight = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].minResolution;
    dwSfcMaxHeight = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].maxResolution;
    fScaleMin      = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].minScalingRatio;
    fScaleMax      = m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].maxScalingRatio;
    if (m_hwCaps.m_rules.sfcMultiPassSupport.scaling.enable)
    {
        fScaleMin2Pass = fScaleMin * m_hwCaps.m_rules.sfcMultiPassSupport.scaling.downScaling.minRatioEnlarged;
        fScaleMax2Pass = fScaleMax * m_hwCaps.m_rules.sfcMultiPassSupport.scaling.upScaling.maxRatioEnlarged;
    }

    if (scalingParams->interlacedScalingType == ISCALING_FIELD_TO_INTERLEAVED)
    {
        dwDstMinHeight = dwSfcMinHeight * 2;
    }
    else
    {
        dwDstMinHeight = dwSfcMinHeight;
    }

    dwSurfaceWidth  = scalingParams->input.dwWidth;
    dwSurfaceHeight = scalingParams->input.dwHeight;
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

    // Calculate the scaling ratio
    // Both source region and scaled region are pre-rotated
    // Need to take Interlace scaling into consideration next step
    fScaleX = (float)dwOutputRegionWidth / (float)dwSourceRegionWidth;
    fScaleY = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;

    if (fScaleX == 1.0f && fScaleY == 1.0f &&
        // Only support vebox crop from left-top, which is to align with legacy path.
        0 == scalingParams->input.rcSrc.left && 0 == scalingParams->input.rcSrc.top &&
        SAME_SIZE_RECT(scalingParams->input.rcDst, scalingParams->output.rcDst) &&
        // If Alpha enabled, which should go SFC pipe.
        !IsAlphaEnabled(scalingParams) &&
        // If Colorfill enabled, which should go SFC pipe.
        !IsColorfillEnabled(scalingParams) &&
        scalingParams->interlacedScalingType != ISCALING_INTERLEAVED_TO_FIELD &&
        scalingParams->interlacedScalingType != ISCALING_FIELD_TO_INTERLEAVED)
    {
        // for non-Scaling cases, all engine supported
        scalingEngine->bEnabled             = 0;
        scalingEngine->SfcNeeded            = 0;
        scalingEngine->VeboxNeeded          = 0;
        scalingEngine->RenderNeeded         = 0;
        scalingEngine->forceEnableForSfc    = 1;
        return MOS_STATUS_SUCCESS;
    }

    // SFC Scaling enabling check
    if (m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].inputSupported   &&
        m_hwCaps.m_sfcHwEntry[scalingParams->formatOutput].outputSupported &&
        m_hwCaps.m_sfcHwEntry[scalingParams->formatInput].scalingSupported)
    {
        if (!(OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
              OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
              OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
              OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
              OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
              OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
              OUT_OF_BOUNDS(dwOutputSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)   ||
              OUT_OF_BOUNDS(dwOutputSurfaceHeight, dwDstMinHeight, dwSfcMaxHeight)))
        {
            if ((m_hwCaps.m_rules.sfcMultiPassSupport.scaling.enable            &&
                (OUT_OF_BOUNDS(fScaleX, fScaleMin2Pass, fScaleMax2Pass)         ||
                 OUT_OF_BOUNDS(fScaleY, fScaleMin2Pass, fScaleMax2Pass)))       ||
                (!m_hwCaps.m_rules.sfcMultiPassSupport.scaling.enable           &&
                (OUT_OF_BOUNDS(fScaleX, fScaleMin, fScaleMax)                   ||
                 OUT_OF_BOUNDS(fScaleY, fScaleMin, fScaleMax)))                 ||
                (scalingParams->scalingPreference == VPHAL_SCALING_PREFER_COMP))
            {
                // Render Pipe, need to add more conditions next step for multiple SFC mode
                // if Render didn't have AVS but Scaling quality mode needed
                scalingEngine->bEnabled     = 1;
                scalingEngine->RenderNeeded = 1;
                scalingEngine->fcSupported  = 1;
                scalingEngine->SfcNeeded    = 0;
            }
            // SFC feasible
            else
            {
                bool sfc2PassScalingNeededX = OUT_OF_BOUNDS(fScaleX, fScaleMin, fScaleMax);
                bool sfc2PassScalingNeededY = OUT_OF_BOUNDS(fScaleY, fScaleMin, fScaleMax);

                scalingEngine->bEnabled = 1;
                if (scalingParams->isPrimary)
                {
                    // For primary layer, force to use sfc for better quailty.
                    scalingEngine->SfcNeeded = 1;
                    scalingEngine->sfc2PassScalingNeededX = sfc2PassScalingNeededX ? 1 : 0;
                    scalingEngine->sfc2PassScalingNeededY = sfc2PassScalingNeededY ? 1 : 0;
                    scalingEngine->multiPassNeeded = sfc2PassScalingNeededX || sfc2PassScalingNeededY;
                }
                else
                {
                    scalingEngine->RenderNeeded = 1;
                    scalingEngine->fcSupported  = 1;
                    // For non-primary layer, only consider sfc for non-2-pass case.
                    if (!sfc2PassScalingNeededX && !sfc2PassScalingNeededY)
                    {
                        scalingEngine->SfcNeeded = 1;
                    }
                }
            }
        }
    }

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

    SwFilterRotMir* rotation = (SwFilterRotMir*)feature;

    FeatureParamRotMir *rotationParams = &rotation->GetSwFilterParams();

    VP_EngineEntry *rotationEngine = &rotation->GetFilterEngineCaps();

    if (rotationEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");
        // Clean usedForNextPass flag.
        if (rotationEngine->usedForNextPass)
        {
            rotationEngine->usedForNextPass = false;
        }
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
        return MOS_STATUS_SUCCESS;
    }

    rotationEngine->bEnabled        = 1;
    rotationEngine->RenderNeeded    = 1;
    rotationEngine->fcSupported     = 1;
    rotationEngine->SfcNeeded       = IsSfcRotationSupported(rotationParams);

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
        return MOS_STATUS_SUCCESS;
    }

    if (m_hwCaps.m_veboxHwEntry[inputformat].denoiseSupported)
    {
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

        if (MOS_IS_ALIGNED(denoiseParams.heightInput, heightAlignUnit))
        {
            denoiseEngine.bEnabled    = 1;
            denoiseEngine.VeboxNeeded = 1;
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("Denoise Feature is disabled since heightInput (%d) not being %d aligned.", denoiseParams.heightInput, heightAlignUnit);
        }
    }

    denoiseParams.widthAlignUnitInput = widthAlignUint;
    denoiseParams.heightAlignUnitInput = heightAlignUnit;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetDeinterlaceExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterDeinterlace* swFilterDi = dynamic_cast<SwFilterDeinterlace*>(feature); 
    VP_PUBLIC_CHK_NULL_RETURN(swFilterDi);

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
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");
        return MOS_STATUS_SUCCESS;
    }

    if (m_vpInterface.GetResourceManager()->IsRefValid() &&
        diParams.diParams && diParams.diParams->bEnableFMD)
    {
        diParams.bFmdExtraVariance = true;
    }

    if (m_vpInterface.GetResourceManager()->IsRefValid()    &&
        m_vpInterface.GetResourceManager()->IsSameSamples())
    {
        diEngine.bypassVeboxFeatures    = 1;
        diEngine.diProcess2ndField      = 1;
    }
    else if (diParams.bFmdExtraVariance && diParams.bFmdKernelEnable)
    {
        diEngine.bEnabled     = 1;
        diEngine.RenderNeeded = 1;
        diEngine.isolated     = 1;
    }
    else if (m_hwCaps.m_veboxHwEntry[inputformat].deinterlaceSupported)
    {
        diEngine.bEnabled     = 1;
        diEngine.VeboxNeeded  = 1;
        diEngine.RenderNeeded = 1;
        diEngine.fcSupported  = 1;
    }
    else
    {
        diEngine.bEnabled     = 1;
        diEngine.RenderNeeded = 1;
        diEngine.fcSupported  = 1;
    }

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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetProcampExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterProcamp* procampFilter = dynamic_cast<SwFilterProcamp*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(procampFilter);

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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetHdrExecutionCaps(SwFilter *feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterHdr *hdrFilter = dynamic_cast<SwFilterHdr *>(feature);

    FeatureParamHdr *hdrParams = &hdrFilter->GetSwFilterParams();

    VP_EngineEntry *pHDREngine  = &hdrFilter->GetFilterEngineCaps();
    MOS_FORMAT      inputformat = hdrParams->formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (pHDREngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("HDR Feature Already been processed, Skip further process");
        return MOS_STATUS_SUCCESS;
    }

    if (m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].inputSupported &&
        m_hwCaps.m_veboxHwEntry[hdrParams->formatInput].hdrSupported)
    {
        pHDREngine->bEnabled        = 1;
        pHDREngine->VeboxNeeded     = 1;
        if (hdrParams->formatOutput == Format_A8B8G8R8 || hdrParams->formatOutput == Format_A8R8G8B8)
        {
            pHDREngine->VeboxARGBOut = 1;
        }
        else if (hdrParams->formatOutput == Format_B10G10R10A2 || hdrParams->formatOutput == Format_R10G10B10A2)
        {
            pHDREngine->VeboxARGB10bitOutput = 1;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetColorFillExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterColorFill* filter = dynamic_cast<SwFilterColorFill*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(filter);

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
        return MOS_STATUS_SUCCESS;
    }

    engine.bEnabled = 1;
    engine.RenderNeeded = 1;
    engine.fcSupported = 1;
    engine.SfcNeeded = 1;    // For SFC, the parameter in scaling is used.

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetAlphaExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterAlpha* filter = dynamic_cast<SwFilterAlpha*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(filter);

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
        return MOS_STATUS_SUCCESS;
    }

    engine.bEnabled = 1;
    engine.RenderNeeded = 1;
    engine.fcSupported = 1;
    engine.SfcNeeded = 1;    // For SFC, the parameter in scaling is used.
    engine.VeboxNeeded = params.compAlpha->AlphaMode != VPHAL_ALPHA_FILL_MODE_BACKGROUND;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(feature);

    VP_EngineEntry &defaultEngine = feature->GetFilterEngineCaps();
    defaultEngine.value = 0;

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
        if (engineCapsInputPipe.VeboxNeeded != 0 || engineCapsInputPipe.SfcNeeded != 0)
        {
            caps.bVebox = true;
            caps.bIECP = engineCapsInputPipe.VeboxIECPNeeded;
            caps.bSFC = engineCapsInputPipe.SfcNeeded != 0;
        }
        else if (engineCapsInputPipe.RenderNeeded)
        {
            caps.bRender = 1;
        }
        else
        {
            // No valid engine selected.
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
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
        caps.bSFC = engineCaps.nonVeboxFeatureExists;

        caps.bDiProcess2ndField = engineCaps.diProcess2ndField;
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
        if (!engineCaps.fcOnlyFeatureExists)
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
    }

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

    return MOS_STATUS_SUCCESS;
}

bool Policy::IsExcludedFeatureForHdr(FeatureType feature)
{
    return (FeatureTypeTcc  == feature  ||
        FeatureTypeSte      == feature  ||
        FeatureTypeProcamp  == feature);
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
                continue;
            }

            if (engineCaps.isolated)
            {
                // 1. Process feature with isolated flag firstly.
                isSingleSubPipe = true;
                selectedPipeIndex = pipeIndex;
                singlePipeSelected = featureSubPipe;
                engineCapsIsolated = engineCaps;
                break;
            }
            else if (!engineCaps.fcSupported)
            {
                // 2. Process non-fc features by vebox/sfc.
                if (engineCaps.RenderNeeded)
                {
                    // Non-FC render feature should be isolated.
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
                }
                if (!engineCaps.SfcNeeded && !engineCaps.VeboxNeeded)
                {
                    // Invalid feature with no engine enabled.
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
                }

                isSingleSubPipe = true;
                selectedPipeIndex = pipeIndex;
                singlePipeSelected = featureSubPipe;
                engineCapsForVeboxSfc.value |= engineCaps.value;
                engineCapsForVeboxSfc.nonFcFeatureExists = true;
                engineCapsForVeboxSfc.nonVeboxFeatureExists |= !engineCaps.VeboxNeeded;
            }
            else
            {
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
                                        featurePipe.GetSurfaceCount(true) == 0;

    engineCapsInputPipe = engineCapsIsolated.isolated ? engineCapsIsolated :
                        engineCapsForVeboxSfc.nonFcFeatureExists ? engineCapsForVeboxSfc :
                        engineCapsForFc;

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

MOS_STATUS Policy::FilterFeatureCombination(SwFilterSubPipe *pipe)
{
    VP_FUNC_CALL();

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
                }
            }
            if (filterID == FeatureTypeCsc)
            {
                SwFilterCsc *feature = (SwFilterCsc *)pipe->GetSwFilter(FeatureType(filterID));
                if (feature)
                {
                    auto &params      = feature->GetSwFilterParams();
                    params.pIEFParams = nullptr;
                }
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecuteFilter(SwFilterPipe& featurePipe, std::vector<int> &layerIndexes, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    params.Type = EngineTypeInvalid;
    params.vpExecuteCaps = caps;

    VP_PUBLIC_CHK_STATUS_RETURN(SetupExecuteFilter(featurePipe, layerIndexes, caps, params));

    // Build Execute surface needed
    VP_PUBLIC_CHK_STATUS_RETURN(SetupFilterResource(featurePipe, layerIndexes, caps, params));

    VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.Update());
    VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->Update());

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteHwFilter(caps, params));

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
                    return MOS_STATUS_NO_SPACE;
                }
            }
        }
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
                    engineCaps->bEnabled = 1;
                    engineCaps->VeboxNeeded = 1;
                }

                VP_PUBLIC_CHK_STATUS_RETURN(UpdateExeCaps(feature, caps, EngineTypeVebox));
            }
            else if (caps.bRender                   &&
                (engineCaps->forceEnableForRender ||
                engineCaps->bEnabled && engineCaps->RenderNeeded))
            {
                if (engineCaps->forceEnableForRender)
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
    if (isSingleSubPipe && !caps.bComposite)
    {
        layerIndexes.push_back(pipeIndex);
        return MOS_STATUS_SUCCESS;
    }

    if (!caps.bComposite)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }

    return MOS_STATUS_UNIMPLEMENTED;
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
                PolicyFeatureHandler *handler = m_VeboxSfcFeatureHandlers.find(feature->GetFeatureType())->second;
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
                PolicyFeatureHandler *handler = m_RenderFeatureHandlers.find(feature->GetFeatureType())->second;
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
                PolicyFeatureHandler *handler = handlers->find(feature->GetFeatureType())->second;
                VP_PUBLIC_CHK_STATUS_RETURN(handler->UpdateUnusedFeature(caps, *feature, featurePipe, executedFilters, isInputPipe, executePipeIndex));
            }

            if (!engineCaps->bEnabled)
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

    if (!isInputPipe && featureSelected && !featureSubPipe->IsEmpty())
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
        VPHAL_PUBLIC_ASSERTMESSAGE("bOutputPipeFeatureInuse being set but input pipe is not empty.");
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
    }
    else if (caps.bComposite)
    {
        // For FC, always output to output surface.
        // If still layers not processed, keep output surface also in featurePipe.
        surfOutput = featurePipe.GetSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
        VP_SURFACE *surfOutput2 = m_vpInterface.GetAllocator().AllocateVpSurface(*surfOutput);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput2);
        VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfOutput2, false, 0));
    }
    else if (RenderTargetTypeParameter == featurePipe.GetRenderTargetType())
    {
        surfOutput = featurePipe.GetSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
        VP_SURFACE *surfOutput2 = m_vpInterface.GetAllocator().AllocateVpSurface(*surfOutput);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput2);
        VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfOutput2, false, 0));
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
    else if (subPipe && RenderTargetTypeParameter == subPipe->GetRenderTargetType())
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
        for (i = 0; i < params.executedFilters->GetSurfaceCount(true); ++i)
        {
            uint32_t index = params.executedFilters->GetLinkedLayerIndex(i);
            // multi-pass with intermedia surface case.
            VP_PUBLIC_NORMALMESSAGE("Output is not empty, featurePipe.IsPrimaryEmpty() = %d", featurePipe.IsPrimaryEmpty());
            VP_SURFACE *intermediaSurface = params.executedFilters->GetSurface(false, i);
            VP_PUBLIC_CHK_NULL_RETURN(intermediaSurface);
            VP_SURFACE *input =  m_vpInterface.GetAllocator().AllocateVpSurface(*intermediaSurface);
            VP_PUBLIC_CHK_NULL_RETURN(input);
            input->SurfType = SURF_IN_PRIMARY;
            featurePipe.ReplaceSurface(input, true, index);
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
        case FeatureTypeAce:
            caps.bACE = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Ace, Vebox)));
            break;
        case FeatureTypeTcc:
            caps.bTCC = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Tcc, Vebox)));
            break;
        case FeatureTypeProcamp:
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
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Hdr, Vebox)));
            break;
        case FeatureTypeLace:
            caps.bLACE = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Lace, Vebox)));
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
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Csc, Render)));
            break;
        case FeatureTypeScaling:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Scaling, Render)));
            break;
        case FeatureTypeRotMir:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(RotMir, Render)));
            break;
        case FeatureTypeSR:
            caps.bSR = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(SR, Render)));
        case FeatureTypeLace:
            caps.bLACE = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Lace, Render)));
            break;
        case FeatureTypeDi:
            caps.bDI          = 1;
            if (feature->GetFilterEngineCaps().isolated)
            {
                caps.bDIFmdKernel = 1;
                feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(DiFmd, Render)));
            }
            else
            {
                feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Di, Render)));
            }
            break;
        case FeatureTypeColorFill:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(ColorFill, Sfc)));
            break;
        case FeatureTypeAlpha:
            caps.bComposite = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Alpha, Sfc)));
            break;
        default:
            break;
        }
    }

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
    if (!caps.bBeCSC && ((caps.bSFC && (caps.bIECP || caps.bDI)) || (!caps.bSFC && caps.b3DlutOutput)))
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
        VP_PUBLIC_CHK_STATUS_RETURN(GetCscParamsOnCaps(pSurfInput, pSurfOutput, caps, cscParams));

        status = csc->Configure(cscParams);
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

namespace vp
{
MOS_STATUS GetVeboxOutputParams(VP_EXECUTE_CAPS &executeCaps, MOS_FORMAT inputFormat, MOS_TILE_TYPE inputTileType, MOS_FORMAT outputFormat, MOS_FORMAT &veboxOutputFormat, MOS_TILE_TYPE &veboxOutputTileType);
}

MOS_STATUS Policy::GetCscParamsOnCaps(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS &caps, FeatureParamCsc &cscParams)
{
    if (caps.bHDR3DLUT)
    {
        cscParams.input.colorSpace  = surfInput->ColorSpace;
        cscParams.formatInput       = surfInput->osSurface->Format;
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

        GetVeboxOutputParams(caps, surfInput->osSurface->Format, surfInput->osSurface->TileType, surfOutput->osSurface->Format, veboxOutputFormat, veboxOutputTileType);
        cscParams.input.colorSpace = surfInput->ColorSpace;
        cscParams.output.colorSpace = surfInput->ColorSpace;

        cscParams.formatInput        = surfInput->osSurface->Format;
        cscParams.formatOutput       = veboxOutputFormat;
        cscParams.input.chromaSiting = surfInput->ChromaSiting;
        cscParams.output.chromaSiting = surfOutput->ChromaSiting;

        cscParams.pAlphaParams = nullptr;
        cscParams.pIEFParams   = nullptr;

        return MOS_STATUS_SUCCESS;
    }

    return MOS_STATUS_UNIMPLEMENTED;
}
