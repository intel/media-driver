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
    VP_PUBLIC_CHK_STATUS_RETURN(vpPlatformInterface->InitVpVeboxSfcHwCaps(m_veboxHwEntry, Format_Count, m_sfcHwEntry, Format_Count));
    // Place hold for render hw caps.
    // VP_PUBLIC_CHK_STATUS_RETURN(vpPlatformInterface->InitVpRenderHwCaps());
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

    p = MOS_New(PolicyVeboxDnHandler);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeDnOnVebox, p));

    p = MOS_New(PolicyVeboxCscHandler);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeCscOnVebox, p));

    p = MOS_New(PolicyVeboxAceHandler);
    VP_PUBLIC_CHK_NULL_RETURN(p);
    m_VeboxSfcFeatureHandlers.insert(std::make_pair(FeatureTypeAceOnVebox, p));

    // Next step to add a table to trace all SW features based on platforms
    m_featurePool.clear();
    m_featurePool.push_back(FeatureTypeCsc);
    m_featurePool.push_back(FeatureTypeScaling);
    m_featurePool.push_back(FeatureTypeRotMir);
    m_featurePool.push_back(FeatureTypeDn);
    m_featurePool.push_back(FeatureTypeAce);

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
        case FeatureTypeDn:
            GetDenoiseExecutionCaps(feature);
            break;
        case FeatureTypeAce:
            GetAceExecutionCaps(feature);
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

    if (m_bypassCompMode             != VP_COMP_BYPASS_DISABLED       &&
        cscParams->formatInput       == cscParams->formatOutput       &&
        cscParams->colorSpaceInput   == cscParams->colorSpaceOutput   &&
        cscParams->chromaSitingInput == cscParams->chromaSitingOutput &&
        nullptr                      == cscParams->pIEFParams)
    {
        cscEngine->bEnabled     = 1;
        cscEngine->SfcNeeded    = 0;
        cscEngine->VeboxNeeded  = 0;
        cscEngine->RenderNeeded = 0;

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

    // if vebox bypass composition mode disabled, then such mega feature like CSC will not take effect in Vebox.
    // then CSC will must be assign to SFC/Render path for execution.
    if (m_bypassCompMode != VP_COMP_BYPASS_DISABLED                              &&
        !cscParams->pIEFParams                                                   &&
       (!cscParams->pAlphaParams                                                 ||
         cscParams->pAlphaParams->AlphaMode != VPHAL_ALPHA_FILL_MODE_BACKGROUND) &&
        m_veboxHwEntry[cscParams->formatInput].inputSupported                    &&
        m_veboxHwEntry[cscParams->formatOutput].outputSupported                  &&
        m_veboxHwEntry[cscParams->formatInput].iecp                              &&
        m_veboxHwEntry[cscParams->formatInput].backEndCscSupported)
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

    if (fScaleX == 1.0f &&
        fScaleY == 1.0f)
    {
        // for non-Scaling cases, all engine supported
        scalingEngine->bEnabled     = 1;
        scalingEngine->SfcNeeded    = 0;
        scalingEngine->VeboxNeeded  = 0;
        scalingEngine->RenderNeeded = 0;
        return MOS_STATUS_SUCCESS;
    }

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
                OUT_OF_BOUNDS(fScaleY, fScaleMin, fScaleMax) ||
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
        rotationEngine->bEnabled     = 1;
        rotationEngine->VeboxNeeded  = 0;
        rotationEngine->SfcNeeded    = 0;
        rotationEngine->RenderNeeded = 0;
        return MOS_STATUS_SUCCESS;
    }

    // SFC Rotation/Mirror enabling check
    if (m_sfcHwEntry[rotationParams->formatInput].inputSupported &&
        m_sfcHwEntry[rotationParams->formatOutput].outputSupported)
    {
        if (rotationParams->rotation > VPHAL_ROTATION_270               &&
            (!m_sfcHwEntry[rotationParams->formatInput].mirrorSupported ||
             rotationParams->tileOutput != MOS_TILE_Y))
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

    SwFilterDenoise* denoise = (SwFilterDenoise*)feature;

    FeatureParamDenoise* denoiseParams = &denoise->GetSwFilterParams();

    VP_EngineEntry* denoiseEngine = &denoise->GetFilterEngineCaps();
    MOS_FORMAT      inputformat = denoiseParams->formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    uint32_t        widthAlignUint  = m_veboxHwEntry[inputformat].horizontalAlignUnit;
    uint32_t        heightAlignUnit = m_veboxHwEntry[inputformat].verticalAlignUnit;

    if (denoiseEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("Scaling Feature Already been processed, Skip further process");
        return MOS_STATUS_SUCCESS;
    }

    if (m_veboxHwEntry[inputformat].denoiseSupported)
    {
        widthAlignUint = MOS_ALIGN_CEIL(m_veboxHwEntry[inputformat].horizontalAlignUnit, 2);

        if (inputformat == Format_NV12 ||
            inputformat == Format_P010 ||
            inputformat == Format_P016)
        {
            heightAlignUnit = MOS_ALIGN_CEIL(m_veboxHwEntry[inputformat].verticalAlignUnit, 4);
        }
        else
        {
            heightAlignUnit = MOS_ALIGN_CEIL(m_veboxHwEntry[inputformat].verticalAlignUnit, 2);
        }

        if (MOS_IS_ALIGNED(denoiseParams->heightInput, heightAlignUnit))
        {
            denoiseEngine->bEnabled    = 1;
            denoiseEngine->VeboxNeeded = 1;
        }
        else
        {
            VP_PUBLIC_NORMALMESSAGE("Denoise Feature is disabled since heightInput (%d) not being %d aligned.", denoiseParams->heightInput, heightAlignUnit);
        }
    }

    denoiseParams->widthAlignUnitInput = widthAlignUint;
    denoiseParams->heightAlignUnitInput = heightAlignUnit;

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetAceExecutionCaps(SwFilter* feature)
{
    VP_FUNC_CALL();
    VP_PUBLIC_CHK_NULL_RETURN(feature);

    SwFilterAce* aceFilter = (SwFilterAce*)feature;

    FeatureParamAce* pAceParams = &aceFilter->GetSwFilterParams();

    VP_EngineEntry* pAceEngine = &aceFilter->GetFilterEngineCaps();
    MOS_FORMAT      inputformat = pAceParams->formatInput;

    // MOS_FORMAT is [-14,103], cannot use -14~-1 as index for m_veboxHwEntry
    if (inputformat < 0)
    {
        inputformat = Format_Any;
    }

    if (pAceEngine->value != 0)
    {
        VP_PUBLIC_NORMALMESSAGE("ACE Feature Already been processed, Skip further process");
        return MOS_STATUS_SUCCESS;
    }

    if (m_veboxHwEntry[pAceParams->formatInput].inputSupported &&
        m_veboxHwEntry[pAceParams->formatOutput].outputSupported &&
        m_veboxHwEntry[pAceParams->formatInput].iecp)
    {
        pAceEngine->bEnabled = 1;
        pAceEngine->VeboxNeeded = 1;
        pAceEngine->VeboxIECPNeeded = 1;
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

    VP_PUBLIC_CHK_STATUS_RETURN(BuildExecuteFilter(featurePipe, caps, params));

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
                    featurePipe.RemoveSwFilter(feature);
                    params.executedFilters->AddSwFilterUnordered(feature, true, 0);

                    //Create and Add CSC filter for VEBOX IECP chromasiting config
                    if (caps.bIECP && filterID == FeatureTypeCsc)
                    {
                        AddNewFilterOnVebox(featurePipe, caps, params, FeatureTypeCsc);
                    }
                }
                // Vebox only cases
                else if (caps.bVebox && engineCaps->bEnabled &&
                    (engineCaps->VeboxNeeded || (caps.bIECP && filterID == FeatureTypeCsc)))
                {
                    UpdateExeCaps(feature, caps, EngineTypeVebox);
                    featurePipe.RemoveSwFilter(feature);
                    params.executedFilters->AddSwFilterUnordered(feature, true, 0);
                }
                else if (caps.bComposite && engineCaps->RenderNeeded)
                {
                    // use render path to implement feature.
                    UpdateExeCaps(feature, caps, EngineTypeRender);
                    if (caps.bIECP && filterID == FeatureTypeCsc)
                    {
                        AddNewFilterOnVebox(featurePipe, caps, params, FeatureTypeCsc);
                    }
                }
                else
                {
                    auto handler = m_vpInterface.GetSwFilterHandler(feature->GetFeatureType());

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
    }
    else
    {
        // goto Composition in Render
    }

    /* Place Holder: order pipe need to be insert in next step*/

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS vp::Policy::SetupFilterResource(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_SURFACE* surfInput  = nullptr;
    VP_SURFACE* surfOutput = nullptr;
    uint32_t    index      = 0;
    SwFilterSubPipe* inputPipe = featurePipe.GetSwFilterPrimaryPipe(index);

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

    if (caps.bVebox)
    {
        // Create Vebox Resources
        VP_PUBLIC_CHK_STATUS_RETURN(AllocateVeboxExecuteResource(caps, params));
    }

    if (caps.bSFC)
    {
        // Create Sfc Resources
        VP_PUBLIC_CHK_STATUS_RETURN(AllocateSfcExecuteResource(caps, params));
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
        case FeatureTypeAce:
            caps.bAce = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Ace, Vebox)));
            break;
        case FeatureTypeCsc:
            caps.bBeCSC = 1;
            feature->SetFeatureType(FeatureType(FEATURE_TYPE_EXECUTE(Csc, Vebox)));
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

MOS_STATUS vp::Policy::AllocateVeboxExecuteResource(VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

    VP_SURFACE *inputSurface = params.executedFilters->GetSurface(true, 0);
    VP_SURFACE *outputSurface = params.executedFilters->GetSurface(false, 0);
    m_vpInterface.GetResourceManager()->AllocateVeboxResource(caps, inputSurface, outputSurface);

    params.executedFilters->GetSurfacesGroup().clear();
    // Insert DN output surface
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeDNOutput, m_vpInterface.GetResourceManager()->GetVeboxDNOutputSurface()));
    // Insert DN Reference surface
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeDNRef, m_vpInterface.GetResourceManager()->GetVeboxDNReferenceSurface()));
    // Insert Vebox output surface when DI/IECP enabled if needed
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeVeboxoutput, m_vpInterface.GetResourceManager()->GetVeboxOutputSurface(caps)));
    // Insert STMM input surface
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeSTMMIn, m_vpInterface.GetResourceManager()->GetVeboxSTMMSurfaceIn()));
    // Insert STMM output surface
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeSTMMOut, m_vpInterface.GetResourceManager()->GetVeboxSTMMSurfaceOut()));
    // Insert Vebox statistics surface
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeStatistics, m_vpInterface.GetResourceManager()->GetVeboxStatisticsSurface()));
    // Insert Vebox histogram surface
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeLaceAceRGBHistogram, m_vpInterface.GetResourceManager()->GetVeboxRgbHistogram()));
    // Insert Vebox auto DN noise level surface
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceTypeAutoDNNoiseLevel, m_vpInterface.GetResourceManager()->GetVeboxDNTempSurface()));
    // Insert Vebox auto DN spatial config surface/buffer
    params.executedFilters->GetSurfacesGroup().insert
    (std::make_pair(SurfaceTypeAutoDNSpatialConfig, m_vpInterface.GetResourceManager()->GetVeboxDNSpatialConfigSurface()));
    //Insert Vebox 3D lut table/buffer
    params.executedFilters->GetSurfacesGroup().insert(
        std::make_pair(SurfaceType3dLut, m_vpInterface.GetResourceManager()->GetVebox3DLookUpTables()));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS vp::Policy::AllocateSfcExecuteResource(VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
{
    VP_FUNC_CALL();

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

MOS_STATUS Policy::AddNewFilterOnVebox(
    SwFilterPipe& featurePipe,
    VP_EXECUTE_CAPS& caps,
    HW_FILTER_PARAMS& params,
    FeatureType featureType)
{
    PVP_SURFACE pSurfInput = featurePipe.GetSurface(true, 0);
    VP_PUBLIC_CHK_NULL_RETURN(pSurfInput);

    auto handler = m_vpInterface.GetSwFilterHandler(featureType);

    if (!handler)
    {
        VP_PUBLIC_ASSERTMESSAGE("no Feature Handle, Return Pipe Init Error");
        return MOS_STATUS_INVALID_HANDLE;
    }

    SwFilter* pSwfilter = handler->CreateSwFilter();
    VP_PUBLIC_CHK_NULL_RETURN(pSwfilter);

    MOS_STATUS status = pSwfilter->Configure(pSurfInput, caps);
    if (MOS_FAILED(status))
    {
        handler->Destory(pSwfilter);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }

    FeatureType featureOnVebox = FeatureTypeInvalid;
    switch (featureType)
    {
    case FeatureTypeDn:
        featureOnVebox = FEATURE_TYPE_EXECUTE(Dn, Vebox);
        break;
    case FeatureTypeAce:
        featureOnVebox = FEATURE_TYPE_EXECUTE(Ace, Vebox);
        break;
    case FeatureTypeCsc:
        featureOnVebox = FEATURE_TYPE_EXECUTE(Csc, Vebox);
        break;
    default:
        featureOnVebox = FeatureTypeInvalid;
        break;
    }

    status = pSwfilter->SetFeatureType(featureOnVebox);
    VP_PUBLIC_CHK_STATUS_RETURN(status);

    status = params.executedFilters->AddSwFilterUnordered(pSwfilter, true, 0);
    VP_PUBLIC_CHK_STATUS_RETURN(status);

    return status;
}
