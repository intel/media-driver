/*
* Copyright (c) 2019, Intel Corporation
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

Policy::Policy(bool bBypassSwFilterPipe, VpInterface &vpInterface) : m_bBypassSwFilterPipe(bBypassSwFilterPipe), m_vpInterface(vpInterface)
{
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

    return MOS_STATUS_SUCCESS;
}

/*                                    Enable SwFilterPipe                                           */

MOS_STATUS Policy::CreateHwFilter(SwFilterPipe &subSwFilterPipe, HwFilter *&pFilter)
{
    if (m_bBypassSwFilterPipe)
    {
        pFilter = nullptr;
        return MOS_STATUS_UNIMPLEMENTED;
    }

    if (subSwFilterPipe.IsEmpty())
    {
        pFilter = nullptr;
        return MOS_STATUS_SUCCESS;
    }

    HW_FILTER_PARAMS param = {};

    MOS_STATUS status = GetHwFilterParam(subSwFilterPipe, param);

    if (MOS_FAILED(status))
    {
        return status;
    }

    pFilter = m_vpInterface.GetHwFilterFactory().Create(param);

    ReleaseHwFilterParam(param);

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

MOS_STATUS Policy::GetHwFilterParam(SwFilterPipe &subSwFilterPipe, HW_FILTER_PARAMS &params)
{
    if (m_bBypassSwFilterPipe)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    VP_EXECUTE_CAPS vpExecuteCaps = {};
    VP_PUBLIC_CHK_STATUS_RETURN(GetExecuteCaps(subSwFilterPipe, vpExecuteCaps));

    // Clear params before using it.
    ReleaseHwFilterParam(params);

    params.Type = EngineTypeInvalid;
    params.vpExecuteCaps = vpExecuteCaps;
    params.isSwFilterEnabled = true;

    // Create and clear executedFilters.
    if (params.executedFilters)
    {
        params.executedFilters->Clean();
    }
    else
    {
        VP_PUBLIC_CHK_STATUS_RETURN(m_vpInterface.GetSwFilterPipeFactory().Create(params.executedFilters));
    }

    // Move surfaces from subSwFilterPipe to executedFilters.
    VP_SURFACE * surfInput = subSwFilterPipe.RemoveSurface(true, 0);
    if (surfInput)
    {
        // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
        VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfInput, true, 0));
    }

    VP_SURFACE * surfOutput = subSwFilterPipe.RemoveSurface(false, 0);
    if (surfOutput)
    {
        // surface should be added before swFilters, since empty feature pipe will be allocated accordingly when surface being added.
        VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSurface(surfOutput, false, 0));
    }

    // Move swFilters from subSwFilterPipe to executedFilters.
    FeatureType featuresRegistered[] = {FeatureTypeCsc, FeatureTypeScaling, FeatureTypeRotMir}; 
    FeatureType featuresToApply[] = {FeatureTypeCscOnSfc, FeatureTypeScalingOnSfc, FeatureTypeRotMirOnSfc};
    int featureCount = sizeof(featuresRegistered) / sizeof(featuresRegistered[0]);

    for (int i = 0; i < featureCount; ++i)
    {
        SwFilter *swFilter = subSwFilterPipe.GetSwFilter(true, 0, featuresRegistered[i]);
        if (swFilter)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(subSwFilterPipe.RemoveSwFilter(swFilter));
            VP_PUBLIC_CHK_STATUS_RETURN(swFilter->SetFeatureType(featuresToApply[i]));
            VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->AddSwFilterUnordered(swFilter, true, 0));
        }
    }

    // Update both subSwFilterPipe and executedFilters.
    VP_PUBLIC_CHK_STATUS_RETURN(subSwFilterPipe.Update());
    VP_PUBLIC_CHK_STATUS_RETURN(params.executedFilters->Update());

    // Create HwFilter parameters for each feature.
    if (vpExecuteCaps.bVebox || vpExecuteCaps.bSFC)
    {
        params.Type = vpExecuteCaps.bSFC ? EngineTypeSfc : EngineTypeVebox;
        std::map<FeatureType, PolicyFeatureHandler*>::iterator it = m_VeboxSfcFeatureHandlers.begin();
        for (; it != m_VeboxSfcFeatureHandlers.end(); ++it)
        {
            if ((*(it->second)).IsFeatureEnabled(vpExecuteCaps))
            {
                HwFilterParameter *pHwFilterParam = (*(it->second)).CreateHwFilterParam(vpExecuteCaps, *params.executedFilters, m_vpInterface.GetHwInterface());
                VP_PUBLIC_CHK_NULL_RETURN(pHwFilterParam);
                params.Params.push_back(pHwFilterParam);
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

/*                                    Disable SwFilterPipe                                          */

MOS_STATUS Policy::CreateHwFilter(VP_PIPELINE_PARAMS &pipelineParams, HwFilter *&pFilter)
{
    if (!m_bBypassSwFilterPipe)
    {
        pFilter = nullptr;
        return MOS_STATUS_UNIMPLEMENTED;
    }
    HW_FILTER_PARAMS param = {};

    VP_PUBLIC_CHK_STATUS_RETURN(GetHwFilterParam(pipelineParams, param));

    pFilter = m_vpInterface.GetHwFilterFactory().Create(param);

    ReleaseHwFilterParam(param);

    VP_PUBLIC_CHK_NULL_RETURN(pFilter);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetExecuteCaps(VP_PIPELINE_PARAMS &pipelineParams, VP_EXECUTE_CAPS &caps)
{
    caps.value = 0;
    // Hardcode now. Will set related value according to pipelineParams later.
    caps.bSFC = 1;
    caps.bSfcCsc = pipelineParams.uSrcCount > 0 && pipelineParams.uDstCount > 0;
    caps.bSfcRotMir = pipelineParams.uSrcCount > 0 && pipelineParams.uDstCount > 0;
    caps.bSfcScaling = pipelineParams.uSrcCount > 0 && pipelineParams.uDstCount > 0;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS Policy::GetHwFilterParam(VP_PIPELINE_PARAMS &pipelineParams, HW_FILTER_PARAMS &params)
{
    if (!m_bBypassSwFilterPipe)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    VP_EXECUTE_CAPS vpExecuteCaps = {};
    VP_PUBLIC_CHK_STATUS_RETURN(GetExecuteCaps(pipelineParams, vpExecuteCaps));

    // Clear params before using it.
    ReleaseHwFilterParam(params);

    params.Type = EngineTypeInvalid;
    params.vpExecuteCaps = vpExecuteCaps;
    params.isSwFilterEnabled = false;
    params.pVpParams = &pipelineParams;
    if (vpExecuteCaps.bVebox || vpExecuteCaps.bSFC)
    {
        params.Type = vpExecuteCaps.bSFC ? EngineTypeSfc : EngineTypeVebox;
        std::map<FeatureType, PolicyFeatureHandler*>::iterator it = m_VeboxSfcFeatureHandlers.begin();
        for (; it != m_VeboxSfcFeatureHandlers.end(); ++it)
        {
            if ((*(it->second)).IsFeatureEnabled(vpExecuteCaps))
            {
                HwFilterParameter *pHwFilterParam = (*(it->second)).CreateHwFilterParam(vpExecuteCaps, pipelineParams, m_vpInterface.GetHwInterface());
                VP_PUBLIC_CHK_NULL_RETURN(pHwFilterParam);
                params.Params.push_back(pHwFilterParam);
            }
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
