/*
* Copyright (c) 2019-2020, Intel Corporation
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
using namespace vp;

/****************************************************************************************************/
/*                                      Policy                                                      */
/****************************************************************************************************/

Policy::Policy(VpInterface &vpInterface) : m_vpInterface(vpInterface)
{
#include "vp_feature_caps_g12.h"
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
    return RegisterFeatures();
}

MOS_STATUS Policy::RegisterFeatures()
{
    VP_FUNC_CALL();
    // Vebox/Sfc features.
    PolicyFeatureHandler *p = MOS_New(PolicySfcCscHandler);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeCscOnSfc, p));

    p = MOS_New(PolicySfcRotMirHandler);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeRotMirOnSfc, p));

    p = MOS_New(PolicySfcScalingHandler);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeScalingOnSfc, p));

    // Next step to add a table to trace all SW features based on platforms
    m_featurePool.clear();
    m_featurePool.push_back(FeatureTypeCsc);
    m_featurePool.push_back(FeatureTypeScaling);
    m_featurePool.push_back(FeatureTypeRotMir);

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

MOS_STATUS Policy::GetExecuteCaps(SwFilterPipe &subSwFilterPipe, VP_EXECUTE_CAPS &caps)
{
    caps.value = 0;
    // Hardcode now. Will set related value according to pipelineParams later.
    caps.bSFC = 1;
    caps.bSfcCsc = SwFilterPipeType1To1 == subSwFilterPipe.GetSwFilterPipeType();
    caps.bSfcRotMir = SwFilterPipeType1To1 == subSwFilterPipe.GetSwFilterPipeType();
    caps.bSfcScaling = SwFilterPipeType1To1 == subSwFilterPipe.GetSwFilterPipeType();
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS vp::Policy::GetHwFilterParam(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params)
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
    uint32_t index;

    VP_PUBLIC_NORMALMESSAGE("Only Support primary layer for advanced processing");
    inputPipe = subSwFilterPipe.GetSwFilterPrimaryPipe(index);

    if (inputPipe)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(BuildExecutionEngines(*inputPipe));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(BuildFilters(subSwFilterPipe, params));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecutionEngines(SwFilterSubPipe& SwFilterPipe)
{
    VP_FUNC_CALL();

    SwFilter* feature = nullptr;
    for (auto filterID : m_featurePool)
    {
        feature = (SwFilter*)SwFilterPipe.GetSwFilter(FeatureType(filterID));

        if (!feature)
        {
            VP_PUBLIC_NORMALMESSAGE("Feature is not enabled in current pipe");
            continue;
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("Feature is enabled in current pipe");
        }

        switch (FeatureType(filterID))
        {
        case FeatureTypeCsc:
            GetCSCExecutionCaps(feature);
            break;
        case FeatureTypeScaling:
            GetScalingExecutionCaps(feature);
            break;
        case FeatureTypeRotMir:
            GetRotationExecutionCaps(feature);
            break;
        default:
            GetExecutionCaps(feature);
            VP_PUBLIC_NORMALMESSAGE("Feature didn't have supported in driver, default to use Render");
            break;
        }

        if (feature->GetFilterEngineCaps().value == 0)
        {
            VP_PUBLIC_ASSERTMESSAGE("ERROR");
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetCSCExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();

    // need to add HDR into consideration in next step
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

    if (IS_COLOR_SPACE_BT2020_YUV(cscParams->colorSpaceInput))
    {
        if ((cscParams->colorSpaceOutput == CSpace_BT601) ||
            (cscParams->colorSpaceOutput == CSpace_BT709) ||
            (cscParams->colorSpaceOutput == CSpace_BT601_FullRange) ||
            (cscParams->colorSpaceOutput == CSpace_BT709_FullRange) ||
            (cscParams->colorSpaceOutput == CSpace_stRGB) ||
            (cscParams->colorSpaceOutput == CSpace_sRGB))
        {
            midFormat = Format_A8R8G8B8;
            cscEngine->VeboxNeeded |= ENGINE_MUST(1); // Vebox Gamut compression is needed
        }
    }

    // SFC CSC enabling check
    if (m_sfcHwEntry[cscParams->formatInput].inputSupported   &&
        m_sfcHwEntry[cscParams->formatOutput].outputSupported &&
        m_sfcHwEntry[cscParams->formatInput].cscSupported)
    {
        if (midFormat != Format_Any &&
            !m_sfcHwEntry[midFormat].cscSupported)
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

    /* Place Holder: Vebox/Render support to be added, Futher process also need to be added */

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetScalingExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();

    uint32_t dwSurfaceWidth, dwSurfaceHeight;
    uint32_t dwOutputSurfaceWidth, dwOutputSurfaceHeight;
    uint32_t dwSfcMinWidth, dwSfcMaxWidth;
    uint32_t dwSfcMinHeight, dwSfcMaxHeight;
    float    fScaleMin, fScaleMax;
    float    fScaleX, fScaleY;

    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterScaling* scaling = (SwFilterScaling*)feature;

    FeatureParamScaling *scalingParams = &scaling->GetSwFilterParams();

    VP_EngineEntry *scalingEngine = &scaling->GetFilterEngineCaps();

    if (scalingEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");
        return MOS_STATUS_SUCCESS;
    }

    dwSfcMinWidth  = m_sfcHwEntry[scalingParams->formatInput].minResolution;
    dwSfcMaxWidth  = m_sfcHwEntry[scalingParams->formatInput].maxResolution;
    dwSfcMinHeight = m_sfcHwEntry[scalingParams->formatInput].minResolution;
    dwSfcMaxHeight = m_sfcHwEntry[scalingParams->formatInput].maxResolution;
    fScaleMin      = m_sfcHwEntry[scalingParams->formatInput].minScalingRatio;
    fScaleMax      = m_sfcHwEntry[scalingParams->formatInput].maxScalingRatio;

    dwSurfaceWidth  = scalingParams->dwWidthInput;
    dwSurfaceHeight = scalingParams->dwHeightInput;
    dwOutputSurfaceWidth  = scalingParams->dwWidthOutput;
    dwOutputSurfaceHeight = scalingParams->dwHeightOutput;

    // Region of the input frame which needs to be processed by SFC
    uint32_t dwSourceRegionHeight = MOS_ALIGN_FLOOR(
        MOS_MIN((uint32_t)(scalingParams->rcSrcInput.bottom - scalingParams->rcSrcInput.top), dwSurfaceHeight),
        m_sfcHwEntry[scalingParams->formatInput].verticalAlignUnit);
    uint32_t dwSourceRegionWidth = MOS_ALIGN_FLOOR(
        MOS_MIN((uint32_t)(scalingParams->rcSrcInput.right - scalingParams->rcSrcInput.left), dwSurfaceWidth),
        m_sfcHwEntry[scalingParams->formatInput].horizontalAlignUnit);

    // Size of the Output Region over the Render Target
    uint32_t dwOutputRegionHeight = MOS_ALIGN_CEIL(
        (uint32_t)(scalingParams->rcDstInput.bottom - scalingParams->rcDstInput.top),
        m_sfcHwEntry[scalingParams->formatOutput].verticalAlignUnit);
    uint32_t dwOutputRegionWidth = MOS_ALIGN_CEIL(
        (uint32_t)(scalingParams->rcDstInput.right - scalingParams->rcDstInput.left),
        m_sfcHwEntry[scalingParams->formatOutput].horizontalAlignUnit);

    // Calculate the scaling ratio
    // Both source region and scaled region are pre-rotated
    // Need to take Interlace scaling into consideration next step
    fScaleX = (float)dwOutputRegionWidth / (float)dwSourceRegionWidth;
    fScaleY = (float)dwOutputRegionHeight / (float)dwSourceRegionHeight;

    // SFC Scaling enabling check
    if (m_sfcHwEntry[scalingParams->formatInput].inputSupported   &&
        m_sfcHwEntry[scalingParams->formatOutput].outputSupported &&
        m_sfcHwEntry[scalingParams->formatInput].scalingSupported)
    {
        if (!(OUT_OF_BOUNDS(dwSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)         ||
              OUT_OF_BOUNDS(dwSurfaceHeight, dwSfcMinHeight, dwSfcMaxHeight)      ||
              OUT_OF_BOUNDS(dwSourceRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
              OUT_OF_BOUNDS(dwSourceRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
              OUT_OF_BOUNDS(dwOutputRegionWidth, dwSfcMinWidth, dwSfcMaxWidth)    ||
              OUT_OF_BOUNDS(dwOutputRegionHeight, dwSfcMinHeight, dwSfcMaxHeight) ||
              OUT_OF_BOUNDS(dwOutputSurfaceWidth, dwSfcMinWidth, dwSfcMaxWidth)   ||
              OUT_OF_BOUNDS(dwOutputSurfaceHeight, dwSfcMinHeight, dwSfcMaxHeight)))
        {
            if (OUT_OF_BOUNDS(fScaleX, fScaleMin, fScaleMax) ||
                OUT_OF_BOUNDS(fScaleY, fScaleMin, fScaleMax))
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

    // SFC CSC enabling check
    if (m_sfcHwEntry[rotationParams->formatInput].inputSupported &&
        m_sfcHwEntry[rotationParams->formatOutput].outputSupported)
    {
        if (m_sfcHwEntry[rotationParams->formatInput].rotationSupported)
        {
            if (rotationParams->rotation > VPHAL_ROTATION_270 &&
                !m_sfcHwEntry[rotationParams->formatInput].mirrorSupported)
            {
                // Render FC Path  for Rotation
                rotationEngine->bEnabled = 1;
                rotationEngine->RenderNeeded = 1;
                rotationEngine->SfcNeeded = 0;
            }
            else
            {
                // SFC Rotation/Mirror can be applied
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
    SwFilterSubPipe* inputPipe = nullptr;
    uint32_t index;

    MOS_ZeroMemory(&caps, sizeof(VP_EXECUTE_CAPS));
    MOS_ZeroMemory(&engineCaps, sizeof(VP_EngineEntry));

    inputPipe = featurePipe.GetSwFilterPrimaryPipe(index);

    if (inputPipe)
    {
        SwFilter* feature = nullptr;
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

    // Sublayer will be processed in render path
    if (!engineCaps.value)
    {
        engineCaps.bEnabled = 1;
        engineCaps.RenderNeeded = 1;
        engineCaps.CompositionNeeded = 1;
    }

    // Enable Vebox Feature only
    bool veboxOnlyEnabled = ENGINE_MUST_MASK(engineCaps.VeboxNeeded) && engineCaps.DisableVeboxSFCMode;

    if (veboxOnlyEnabled)
    {
        // only select Vebox only features
        caps.bVebox = 1;
    }
    else
    {
        caps.bVebox  = (engineCaps.VeboxNeeded != 0);
        caps.bSFC    = (engineCaps.SfcNeeded != 0);

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

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteFilter(featurePipe, caps, params));

    /* Place Holder for Resource Manager to manage intermedia surface or HW needed surface in policy*/

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::BuildExecuteFilter(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    // Clear params before using it.
    VP_PUBLIC_CHK_STATUS_RETURN(ReleaseHwFilterParam(params));

    params.Type = EngineTypeInvalid;
    params.vpExecuteCaps = caps;

    VP_PUBLIC_CHK_STATUS_RETURN(SetupExecuteFilter(featurePipe, caps, params));

    VP_PUBLIC_CHK_STATUS_RETURN(featurePipe.Update());
    VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->Update());

    if (caps.bVebox || caps.bSFC)
    {
        params.Type = caps.bSFC ? EngineTypeSfc : EngineTypeVebox;
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

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::SetupExecuteFilter(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    // Select the Pipe Engine for promary pipe
    uint32_t index;
    SwFilterSubPipe* inputPipe = featurePipe.GetSwFilterPrimaryPipe(index);
    SwFilter* feature = nullptr;
    VP_SURFACE* surfInput = nullptr;
    VP_SURFACE* surfOutput = nullptr;
    VP_EngineEntry *engineCaps;

    // only process Primary surface
    VP_PUBLIC_CHK_NULL_RETURN(inputPipe);

    MOS_ZeroMemory(&engineCaps, sizeof(VP_EngineEntry));

    if (caps.value)
    {
        // Move surfaces from subSwFilterPipe to executedFilters.
        surfInput = featurePipe.GetSurface(true, index);
        if (surfInput)
        {
            // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
            VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfInput, true, index));
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

                if (caps.bSFC && engineCaps->bEnabled && engineCaps->SfcNeeded)
                {
                    /* Place Holder: need to add FurtherProcessNeeded conditions in next step*/
                    // Choose SFC as execution engine
                    UpdateExeCaps(feature, caps, EngineTypeSfc);
                    featurePipe.RemoveSwFilter(feature);
                    params.executedFilters->AddSwFilterUnordered(feature, true, 0);
                }
            }
        }
    }
    else
    {
        // goto Composition in Render
    }

    // modifier the outputpipe params
    if (featurePipe.IsPrimaryEmpty())
    {
        // Update the input feature surfaces
        surfInput = featurePipe.RemoveSurface(true, index);
        surfOutput = featurePipe.RemoveSurface(false, 0);
        if (surfOutput)
        {
            // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
            VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfOutput, false, 0));
        }
    }
    else
    {
        VP_PUBLIC_ASSERTMESSAGE("Output is not empty, featurePipe.IsPrimaryEmpty() = %d", featurePipe.IsPrimaryEmpty());
        /* Place Holder: nest step resource manager will create intermeida surface here for surface sharing b/w packets */
    }

    /* Place Holder: order pipe need to be insert in next step*/

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::UpdateExeCaps(SwFilter* feature, VP_EXECUTE_CAPS& caps, EngineType Type)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    FeatureType featureType = feature->GetFeatureType();

    if (Type == EngineTypeSfc)
    {
        switch (featureType)
        {
        case FeatureTypeCsc:
            caps.bSfcCsc = 1;
            feature->SetFeatureType(FEATURE_TYPE_EXECUTE(Csc, Sfc));
            break;
        case FeatureTypeScaling:
            caps.bSfcScaling = 1;
            feature->SetFeatureType(FEATURE_TYPE_EXECUTE(Scaling, Sfc));
            break;
        case FeatureTypeRotMir:
            caps.bSfcRotMir = 1;
            feature->SetFeatureType(FEATURE_TYPE_EXECUTE(RotMir, Sfc));
            break;
        default:
            break;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::ReleaseHwFilterParam(HW_FILTER_PARAMS &params)
{
    if (EngineTypeInvalid == params.Type || params.Params.empty())
    {
        params.Type = EngineTypeInvalid;
        while (!params.Params.empty())
        {
            HwFilterParameter *p = params.Params.back();
            params.Params.pop_back();
            MOS_Delete(p);
        }
        return MOS_STATUS_SUCCESS;
    }

    std::map<FeatureType, PolicyFeatureHandler*> &featureHandler = 
            (EngineTypeVebox == params.Type || EngineTypeSfc == params.Type) ? m_VeboxSfcFeatureHandlers : m_RenderFeatureHandlers;

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
