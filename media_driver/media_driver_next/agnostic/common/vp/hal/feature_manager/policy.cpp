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
    VpPlatformInterface *vpPlatformInterface = (VpPlatformInterface *)m_vpInterface.GetHwInterface()->m_vpPlatformInterface;
    VP_PUBLIC_CHK_NULL_RETURN(vpPlatformInterface);
    VP_PUBLIC_CHK_STATUS_RETURN(vpPlatformInterface->InitVpHwCaps(m_hwCaps));
    VP_PUBLIC_CHK_STATUS_RETURN(RegisterFeatures());
    m_initialized = true;
    return MOS_STATUS_SUCCESS;
}

bool Policy::IsVeboxSfcFormatSupported(MOS_FORMAT  formatInput, MOS_FORMAT formatOutput)
{
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

    p = MOS_New(PolicyVeboxDiHandler, m_hwCaps);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeDiOnVebox, p));

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
    SwFilterSubPipe* inputPipe = nullptr;
    uint32_t index = 0;

    VP_PUBLIC_NORMALMESSAGE("Only Support primary layer for advanced processing");
    inputPipe = subSwFilterPipe.GetSwFilterPrimaryPipe(index);

    if (inputPipe)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(BuildExecutionEngines(*inputPipe));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(BuildFilters(subSwFilterPipe, params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecutionCapsForSingleFeature(FeatureType featureType, SwFilterSubPipe& swFilterPipe)
{
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
    default:
        VP_PUBLIC_CHK_STATUS_RETURN(GetExecutionCaps(feature));
        VP_PUBLIC_NORMALMESSAGE("Feature didn't have supported in driver, default to use Render");
        break;
    }

    if (feature->GetFilterEngineCaps().value == 0)
    {
        VP_PUBLIC_ASSERTMESSAGE("Error! No engine being assigned!");
        return MOS_STATUS_INVALID_PARAMETER;
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
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetCSCExecutionCapsHdr(SwFilter *HDR, SwFilter *CSC)
{
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

    MOS_FORMAT   hdrFormat = Format_Any;
    VPHAL_CSPACE hdrCSpace = CSpace_Any;

    if (m_hwCaps.m_sfcHwEntry[cscParams->formatInput].inputSupported &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatInput].cscSupported)
    {
        hdrCSpace = IS_COLOR_SPACE_BT2020(cscParams->output.colorSpace) ? CSpace_BT2020_RGB : CSpace_sRGB;
        hdrFormat = IS_COLOR_SPACE_BT2020(cscParams->output.colorSpace) ? Format_R10G10B10A2 : Format_A8R8G8B8;
        if (m_hwCaps.m_sfcHwEntry[hdrFormat].inputSupported &&
            m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
            m_hwCaps.m_sfcHwEntry[hdrFormat].cscSupported)
        {
            cscEngine->bEnabled = 1;
            cscEngine->SfcNeeded |= 1;
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
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterCsc* csc = dynamic_cast<SwFilterCsc*>(feature);
    VP_PUBLIC_CHK_NULL_RETURN(csc);

    VP_PUBLIC_CHK_STATUS_RETURN(GetCSCExecutionCaps(feature));

    VP_EngineEntry *cscEngine = &csc->GetFilterEngineCaps();
    VP_PUBLIC_CHK_NULL_RETURN(cscEngine);
    if (cscEngine->bEnabled &&
        (cscEngine->SfcNeeded && cscEngine->VeboxNeeded ||
        0 == cscEngine->SfcNeeded && 0 == cscEngine->VeboxNeeded && 0 == cscEngine->RenderNeeded))
    {
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
        return MOS_STATUS_SUCCESS;
    }

    if (m_bypassCompMode                != VP_COMP_BYPASS_DISABLED          &&
        cscParams->formatInput          == cscParams->formatOutput          &&
        cscParams->input.colorSpace     == cscParams->output.colorSpace     &&
        cscParams->input.chromaSiting   == cscParams->output.chromaSiting   &&
        nullptr                         == cscParams->pIEFParams)
    {
        // for non-csc cases, all engine supported
        cscEngine->bEnabled     = 1;
        cscEngine->SfcNeeded    = 0;
        cscEngine->VeboxNeeded  = 0;
        cscEngine->RenderNeeded = 0;

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
            midFormat = Format_A8R8G8B8;
            cscEngine->VeboxNeeded |= ENGINE_MUST(1); // Vebox Gamut compression is needed
        }
    }

    // SFC CSC enabling check
    if (m_hwCaps.m_sfcHwEntry[cscParams->formatInput].inputSupported   &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
        m_hwCaps.m_sfcHwEntry[cscParams->formatInput].cscSupported)
    {
        if (midFormat != Format_Any &&
            !m_hwCaps.m_sfcHwEntry[midFormat].cscSupported)
        {
            cscEngine->FurtherProcessNeeded |= 1;
            cscEngine->DisableVeboxSFCMode |= 1;
            //Another CSC Path is needed, need to create a new internal Surface and SW filter(Call SW Filter Factory) for processing
        }
        else
        {
            cscEngine->bEnabled   = 1;
            cscEngine->SfcNeeded |= 1;
        }
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
        cscEngine->bEnabled     = 1;
        cscEngine->VeboxNeeded |= 1;
    }
    /* Place Holder: Render support to be added, Futher process also need to be added */

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
        0 == scalingParams->input.rcSrc.left && 0 == scalingParams->input.rcSrc.top)
    {
        // for non-Scaling cases, all engine supported
        scalingEngine->bEnabled     = 1;
        scalingEngine->SfcNeeded    = 0;
        scalingEngine->VeboxNeeded  = 0;
        scalingEngine->RenderNeeded = 0;
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
                scalingEngine->SfcNeeded    = 0;
            }
            // SFC feasible
            else
            {
                scalingEngine->bEnabled = 1;
                scalingEngine->SfcNeeded = 1;
                if (m_hwCaps.m_rules.sfcMultiPassSupport.scaling.enable)
                {
                    if (OUT_OF_BOUNDS(fScaleX, fScaleMin, fScaleMax))
                    {
                        scalingEngine->sfc2PassScalingNeededX = 1;
                    }
                    if (OUT_OF_BOUNDS(fScaleY, fScaleMin, fScaleMax))
                    {
                        scalingEngine->sfc2PassScalingNeededY = 1;
                    }
                }
            }
        }
    }

    /* Multi-process Scaling need to ba added for futher feature enabling */

    return MOS_STATUS_SUCCESS;
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
        return MOS_STATUS_SUCCESS;
    }

    if (rotationParams->rotation == VPHAL_ROTATION_IDENTITY)
    {
        // for non-rotation cases, all engine supported
        rotationEngine->bEnabled     = 1;
        rotationEngine->VeboxNeeded  = 0;
        rotationEngine->SfcNeeded    = 0;
        rotationEngine->RenderNeeded = 0;
        return MOS_STATUS_SUCCESS;
    }

    // SFC Rotation/Mirror enabling check
    if (m_hwCaps.m_sfcHwEntry[rotationParams->formatInput].inputSupported &&
        m_hwCaps.m_sfcHwEntry[rotationParams->formatOutput].outputSupported)
    {
        if (rotationParams->rotation > VPHAL_ROTATION_270               &&
            (!m_hwCaps.m_sfcHwEntry[rotationParams->formatInput].mirrorSupported ||
             rotationParams->surfInfo.tileOutput != MOS_TILE_Y))
        {
            // Render FC Path  for Rotation
            rotationEngine->bEnabled = 1;
            rotationEngine->RenderNeeded = 1;
            rotationEngine->SfcNeeded = 0;
        }
        else
        {
            rotationEngine->bEnabled = 1;
            rotationEngine->SfcNeeded = 1;
        }
    }
    else
    {
        // Render FC Path  for Rotation
        rotationEngine->bEnabled = 1;
        rotationEngine->RenderNeeded = 1;
        rotationEngine->SfcNeeded = 0;
    }

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

    if (m_vpInterface.GetResourceManager()->IsRefValid()    &&
        m_vpInterface.GetResourceManager()->IsSameSamples())
    {
        diEngine.bypassVeboxFeatures = 1;
    }
    else if (m_hwCaps.m_veboxHwEntry[inputformat].deinterlaceSupported)
    {
        diEngine.bEnabled    = 1;
        diEngine.VeboxNeeded = 1;
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

    if (m_hwCaps.m_veboxHwEntry[inputformat].inputSupported &&
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
        m_hwCaps.m_veboxHwEntry[inputformat].iecp)
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

    if (m_hwCaps.m_veboxHwEntry[inputformat].inputSupported &&
        m_hwCaps.m_veboxHwEntry[inputformat].iecp)
    {
        procampEngine.bEnabled = 1;
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
        pHDREngine->VeboxIECPNeeded = 1;
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

MOS_STATUS Policy::GetExecutionCaps(SwFilter* feature)
{
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    VP_EngineEntry defaultEngine = feature->GetFilterEngineCaps();

    defaultEngine.RenderNeeded = 1;
    defaultEngine.bEnabled     = 1;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildFilters(SwFilterPipe& featurePipe, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_EngineEntry engineCaps;
    VP_EXECUTE_CAPS caps;


    MOS_ZeroMemory(&caps, sizeof(VP_EXECUTE_CAPS));
    MOS_ZeroMemory(&engineCaps, sizeof(VP_EngineEntry));

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateFilterCaps(featurePipe, engineCaps, caps));

    // Sublayer will be processed in render path
    if (!engineCaps.value)
    {
        engineCaps.bEnabled = 1;
        engineCaps.RenderNeeded = 1;
        engineCaps.CompositionNeeded = 1;
    }
    else if (engineCaps.bEnabled && engineCaps.VeboxNeeded == 0 &&
        engineCaps.SfcNeeded == 0 && engineCaps.RenderNeeded == 0)
    {
        // Only scaling/csc/rotation filters and supported by all engines. Select vebox by default.
        engineCaps.bEnabled = 1;
        engineCaps.VeboxNeeded = 1;
        engineCaps.SfcNeeded = 0;
        engineCaps.RenderNeeded = 0;
    }

    // Enable Vebox Feature only
    bool veboxOnlyEnabled = ENGINE_MUST_MASK(engineCaps.VeboxNeeded) && engineCaps.DisableVeboxSFCMode;

    if (veboxOnlyEnabled)
    {
        // only select Vebox only features
        caps.bVebox = 1;
        caps.bIECP = engineCaps.VeboxIECPNeeded;
    }
    else
    {
        caps.bVebox  = ((engineCaps.VeboxNeeded != 0) || (engineCaps.SfcNeeded != 0)) ? 1 : 0;
        caps.bIECP = (caps.bVebox) ? engineCaps.VeboxIECPNeeded : 0;

        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFilterCaps(featurePipe, engineCaps, caps));

        caps.bSFC    = (engineCaps.SfcNeeded != 0) ? 1 : 0;

        if (!caps.bVebox &&
            !caps.bSFC)
        {
            caps.bRender = 1;
        }
        else
        {
            caps.bRender = 0;
        }

        caps.bComposite = (engineCaps.CompositionNeeded != 0);
    }

    if (IsVeboxSecurePathEnabled(featurePipe, caps))
    {
        // Process Vebox Secure workload
        VP_PUBLIC_CHK_STATUS_RETURN(BuildVeboxSecureFilters(featurePipe, caps, params));

        VP_PUBLIC_CHK_STATUS_RETURN(SetupFilterResource(featurePipe, caps, params));

        VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteHwFilter(featurePipe, caps, params));
        return MOS_STATUS_SUCCESS;
    }

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteFilter(featurePipe, caps, params));
    VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.ResetSecureFlag());

    /* Place Holder for Resource Manager to manage intermedia surface or HW needed surface in policy*/

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateFilterCaps(SwFilterPipe& featurePipe, VP_EngineEntry& engineCaps, VP_EXECUTE_CAPS &caps)
{
    VP_FUNC_CALL();

    SwFilterSubPipe* inputPipe = nullptr;
    SwFilter*        feature   = nullptr;
    uint32_t         index     = 0;

    inputPipe = featurePipe.GetSwFilterPrimaryPipe(index);

    // Update the Engine Caps after engine setting up
    if (engineCaps.value !=0 && inputPipe)
    {
        bool sfcNeeded = false;
        // check whether sfc being must have.
        for (auto filterID : m_featurePool)
        {
            feature = inputPipe->GetSwFilter(FeatureType(filterID));

            if (feature)
            {
                if (feature->GetFilterEngineCaps().VeboxNeeded  == 0 &&
                    feature->GetFilterEngineCaps().SfcNeeded    != 0)
                {
                    sfcNeeded = true;
                    break;
                }
            }
        }
        // check the feature pool, generate a workable engine Pipe
        for (auto filterID : m_featurePool)
        {
            feature = inputPipe->GetSwFilter(FeatureType(filterID));

            if (feature)
            {
                if (feature->GetFilterEngineCaps().VeboxNeeded != 0 &&
                    caps.bVebox)
                {
                    if (sfcNeeded && feature->GetFilterEngineCaps().SfcNeeded)
                    {
                        feature->GetFilterEngineCaps().VeboxNeeded  = 0;
                        feature->GetFilterEngineCaps().RenderNeeded = 0;
                    }
                    else
                    {
                        feature->GetFilterEngineCaps().SfcNeeded    = 0;
                        feature->GetFilterEngineCaps().RenderNeeded = 0;
                    }
                }

                if (engineCaps.bypassVeboxFeatures)
                {
                    if (caps.bVebox && feature->GetFilterEngineCaps().VeboxNeeded)
                    {
                        // Disable vebox features.
                        feature->GetFilterEngineCaps().bEnabled = false;
                    }

                    if (FeatureTypeDi == filterID)
                    {
                        caps.bDiProcess2ndField = 1;
                    }
                }
            }
        }
    }

    engineCaps.value = 0;
    // Set Engine Caps the first time
    if (inputPipe)
    {
        // check the feature pool, generate a workable engine Pipe
        for (auto filterID : m_featurePool)
        {
            feature = inputPipe->GetSwFilter(FeatureType(filterID));

            if (feature)
            {
                engineCaps.value |= feature->GetFilterEngineCaps().value;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecuteFilter(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    params.Type = EngineTypeInvalid;
    params.vpExecuteCaps = caps;

    VP_PUBLIC_CHK_STATUS_RETURN(SetupExecuteFilter(featurePipe, caps, params));

    // Build Execute surface needed
    VP_PUBLIC_CHK_STATUS_RETURN(SetupFilterResource(featurePipe, caps, params));

    VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.Update());
    VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->Update());

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteHwFilter(featurePipe, caps, params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecuteHwFilter(SwFilterPipe& subSwFilterPipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
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

MOS_STATUS Policy::SetupExecuteFilter(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    // Select the Pipe Engine for primary pipe
    uint32_t index = 0;
    SwFilterSubPipe* inputPipe = featurePipe.GetSwFilterPrimaryPipe(index);
    SwFilter* feature = nullptr;
    VP_SURFACE* surfInput = nullptr;
    VP_SURFACE* surfOutput = nullptr;
    VP_EngineEntry *engineCaps = nullptr;

    // only process Primary surface
    VP_PUBLIC_CHK_NULL_RETURN(inputPipe);

    if (caps.value)
    {
        // Move surfaces from subSwFilterPipe to executedFilters.
        surfInput = featurePipe.GetSurface(true, index);
        if (surfInput)
        {
            // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
            VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfInput, true, index));
            VP_SURFACE *pastRefSurface = featurePipe.RemovePastSurface(index);
            VP_SURFACE *futureRefSurface = featurePipe.RemoveFutureSurface(index);
            params.executedFilters->SetPastSurface(index, pastRefSurface);
            params.executedFilters->SetFutureSurface(index, futureRefSurface);
        }
        else
        {
            VP_PUBLIC_ASSERTMESSAGE("No input for current pipe");
        }
    }

    if (inputPipe)
    {
        for (auto filterID : m_featurePool)
        {
            feature = (SwFilter*)inputPipe->GetSwFilter(FeatureType(filterID));
            if (feature)
            {
                engineCaps = &(feature->GetFilterEngineCaps());

                 // if SFC enabled, Vebox is must as SFC need connect with Vebox
                if (caps.bSFC && engineCaps->bEnabled && !engineCaps->RenderNeeded)
                {
                    if (!engineCaps->VeboxNeeded && !engineCaps->SfcNeeded)
                    {
                        engineCaps->SfcNeeded = 1;
                    }
                    /* Place Holder: need to add FurtherProcessNeeded conditions in next step*/
                    // Choose SFC as execution engine
                    UpdateExeCaps(feature, caps, engineCaps->SfcNeeded ? EngineTypeVeboxSfc : EngineTypeVebox);
                }
                // Vebox only cases
                else if (caps.bVebox && engineCaps->bEnabled &&
                    (engineCaps->VeboxNeeded || (caps.bIECP && filterID == FeatureTypeCsc)))
                {
                    UpdateExeCaps(feature, caps, EngineTypeVebox);
                }
                else if (caps.bComposite && engineCaps->RenderNeeded)
                {
                    // use render path to implement feature.
                    UpdateExeCaps(feature, caps, EngineTypeRender);
                }
            }
        }

        for (auto filterID : m_featurePool)
        {
            feature = (SwFilter*)inputPipe->GetSwFilter(FeatureType(filterID));
            if (feature)
            {
                engineCaps = &(feature->GetFilterEngineCaps());

                if (m_VeboxSfcFeatureHandlers.end() != m_VeboxSfcFeatureHandlers.find(feature->GetFeatureType()))
                {
                    // Engine has been assigned to feature.
                    PolicyFeatureHandler *handler = m_VeboxSfcFeatureHandlers.find(feature->GetFeatureType())->second;
                    handler->UpdateFeaturePipe(caps, *feature, featurePipe, *params.executedFilters, true, 0);
                }
                else if (!engineCaps->bEnabled || !engineCaps->SfcNeeded && !engineCaps->VeboxNeeded && !engineCaps->RenderNeeded)
                {
                    SwFilterFeatureHandler *handler = m_vpInterface.GetSwFilterHandler(feature->GetFeatureType());

                    if (!handler)
                    {
                        VP_PUBLIC_ASSERTMESSAGE("no Feature Handle, Return Pipe Init Error");
                        return MOS_STATUS_INVALID_HANDLE;
                    }
                    // For feature which is force enabled on Sfc, just drop it if sfc not being used.
                    featurePipe.RemoveSwFilter(feature);
                    handler->Destory(feature);
                    VP_PUBLIC_NORMALMESSAGE("filter missed packets generation");
                }
            }
        }

        VP_PUBLIC_CHK_STATUS_RETURN(AddFiltersBasedOnCaps(featurePipe, caps, *params.executedFilters));
    }
    else
    {
        // goto Composition in Render
    }

    /* Place Holder: order pipe need to be insert in next step*/

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::SetupFilterResource(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_SURFACE* surfInput  = nullptr;
    VP_SURFACE* surfOutput = nullptr;
    uint32_t    index      = 0;
    SwFilterSubPipe* inputPipe = featurePipe.GetSwFilterPrimaryPipe(index);

    if (featurePipe.IsPrimaryEmpty())
    {
        // Update the input feature surfaces
        surfOutput = featurePipe.RemoveSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(surfOutput);
        // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
        VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfOutput, false, 0));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(AssignExecuteResource(caps, params));

    if (featurePipe.IsPrimaryEmpty())
    {
        // Update the input feature surfaces
        surfInput = featurePipe.RemoveSurface(true, index);
    }
    else if (featurePipe.GetSecureProcessFlag())
    {
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateSecureExecuteResource(featurePipe, caps, params));
    }
    else
    {
        VP_PUBLIC_NORMALMESSAGE("Output is not empty, featurePipe.IsPrimaryEmpty() = %d", featurePipe.IsPrimaryEmpty());
        VP_SURFACE *intermediaSurface = params.executedFilters->GetSurface(false, 0);
        VP_PUBLIC_CHK_NULL_RETURN(intermediaSurface);
        VP_SURFACE *input =  m_vpInterface.GetAllocator().AllocateVpSurface(*intermediaSurface);
        VP_PUBLIC_CHK_NULL_RETURN(input);
        input->SurfType = SURF_IN_PRIMARY;
        featurePipe.ReplaceSurface(input, true, index);
    }

    // Place Holder for multi-Process(include FC) cases where Temp surface needed here

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
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Hdr, Vebox)));
            break;
        default:
            break;
        }
    }

    if (Type == EngineTypeRender)
    {
        caps.bComposite = 1;
        switch (featureType)
        {
        case FeatureTypeCsc:
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Csc, Render)));
            break;
        case FeatureTypeScaling:
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Scaling, Render)));
            break;
        case FeatureTypeRotMir:
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(RotMir, Render)));
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

bool Policy::IsVeboxSecurePathEnabled(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps)
{
    VP_FUNC_CALL();

    if (m_vpInterface.GetHwInterface())
    {
        // will remove when secure path ready
        // VP_PUBLIC_ASSERTMESSAGE("No VP Interface Available");
        return false;
    }

    if (!m_vpInterface.GetHwInterface()->m_osInterface ||
        !m_vpInterface.GetHwInterface()->m_osInterface->osCpInterface)
    {
        VP_PUBLIC_ASSERTMESSAGE("No CP Interface Available");
        return false;
    }

    MosCpInterface* cpInterface = (m_vpInterface.GetHwInterface()->m_osInterface->osCpInterface);

    // Place holder: DDI can also have conditions for Kernel resource using
    if (!featurePipe.GetSecureProcessFlag() && caps.bVebox && cpInterface->IsHMEnabled())
    {
        return true;
    }

    if (featurePipe.GetSecureProcessFlag())
    {
        featurePipe.SetSecureProcessFlag(false);
    }

    return false;
}

MOS_STATUS Policy::BuildVeboxSecureFilters(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
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
    VP_EXECUTE_CAPS& caps,
    SwFilterPipe& executedFilters)
{
    //Create and Add CSC filter for VEBOX IECP chromasiting config
    if (caps.bSFC && !caps.bBeCSC && (caps.bIECP || caps.bDI))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AddNewFilterOnVebox(featurePipe, caps, executedFilters, FeatureTypeCsc));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::AddNewFilterOnVebox(
    SwFilterPipe& featurePipe,
    VP_EXECUTE_CAPS& caps,
    SwFilterPipe& executedFilters,
    FeatureType featureType)
{
    PVP_SURFACE pSurfInput = featurePipe.GetSurface(true, 0);
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

    MOS_STATUS status = swfilter->Configure(pSurfInput, pSurfOutput, caps);
    if (MOS_FAILED(status))
    {
        handler->Destory(swfilter);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }

    VP_PUBLIC_CHK_STATUS_RETURN(UpdateExeCaps(swfilter, caps, EngineTypeVebox));

    status = executedFilters.AddSwFilterUnordered(swfilter, true, 0);
    VP_PUBLIC_CHK_STATUS_RETURN(status);

    return status;
}
