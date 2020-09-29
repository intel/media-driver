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
//! \file     sw_filter_pipe.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "sw_filter_pipe.h"
#include "vp_obj_factories.h"
#include "vp_feature_manager.h"
#include "sw_filter_handle.h"

using namespace vp;

/****************************************************************************************************/
/*                                      SwFilterSubPipe                                             */
/****************************************************************************************************/

SwFilterSubPipe::SwFilterSubPipe()
{
}

SwFilterSubPipe::~SwFilterSubPipe()
{
    Clean();
}

MOS_STATUS SwFilterSubPipe::Clean()
{
    for (auto &filterSet : m_OrderedFilters)
    {
        if (filterSet)
        {
            // Loop orderred feature set.
            VP_PUBLIC_CHK_STATUS_RETURN(filterSet->Clean());
            MOS_Delete(filterSet);
        }
    }
    m_OrderedFilters.clear();

    // Process remaining unordered features
    VP_PUBLIC_CHK_STATUS_RETURN(m_UnorderedFilters.Clean());

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSubPipe::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    for (auto &featureSet : m_OrderedFilters)
    {
        if (featureSet)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(featureSet->Update(inputSurf, outputSurf));
        }
    }
    VP_PUBLIC_CHK_STATUS_RETURN(m_UnorderedFilters.Update(inputSurf, outputSurf));

    return MOS_STATUS_SUCCESS;
}

SwFilter *SwFilterSubPipe::GetSwFilter(FeatureType type)
{
    // Search unordered filters firstly.
    SwFilter *swFilter = m_UnorderedFilters.GetSwFilter(type);

    if (swFilter)
    {
        return swFilter;
    }

    for (auto &swFilterSet : m_OrderedFilters)
    {
        swFilter = swFilterSet->GetSwFilter(type);
        if (swFilter)
        {
            return swFilter;
        }
    }

    return nullptr;
}

MOS_STATUS SwFilterSubPipe::AddSwFilterOrdered(SwFilter *swFilter, bool useNewSwFilterSet)
{
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);

    MOS_STATUS status = MOS_STATUS_SUCCESS;
    SwFilterSet *swFilterSet = nullptr;
    auto &pipe = m_OrderedFilters;

    if (useNewSwFilterSet || pipe.empty())
    {
        swFilterSet = MOS_New(SwFilterSet);
        useNewSwFilterSet = true;
    }
    else
    {
        swFilterSet = pipe[pipe.size() - 1];
    }
    VP_PUBLIC_CHK_NULL_RETURN(swFilterSet);

    status = swFilterSet->AddSwFilter(swFilter);

    if (MOS_FAILED(status))
    {
        if (useNewSwFilterSet)
        {
            MOS_Delete(swFilterSet);
        }
        return status;
    }

    pipe.push_back(swFilterSet);
    swFilterSet->SetLocation(&pipe);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSubPipe::AddSwFilterUnordered(SwFilter *swFilter)
{
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);
    return m_UnorderedFilters.AddSwFilter(swFilter);
}

/****************************************************************************************************/
/*                                      SwFilterPipe                                                */
/****************************************************************************************************/

SwFilterPipe::SwFilterPipe(VpInterface &vpInterface) : m_vpInterface(vpInterface)
{
}

SwFilterPipe::~SwFilterPipe()
{
    Clean();
}

MOS_STATUS SwFilterPipe::Initialize(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule)
{
    Clean();

    uint32_t i = 0;
    for (i = 0; i < params.uSrcCount; ++i)
    {
        if (nullptr == params.pSrc[i])
        {
            Clean();
            return MOS_STATUS_INVALID_PARAMETER;
        }
        VP_SURFACE *surf = m_vpInterface.GetAllocator().AllocateVpSurface(*params.pSrc[i]);
        if (nullptr == surf)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_InputSurfaces.push_back(surf);
        // Keep m_pastSurface same size as m_InputSurfaces.
        m_pastSurface.push_back(nullptr);
        m_futureSurface.push_back(nullptr);

        // Initialize m_InputPipes.
        SwFilterSubPipe *pipe = MOS_New(SwFilterSubPipe);
        if (nullptr == pipe)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_InputPipes.push_back(pipe);
    }

    for (i = 0; i < params.uDstCount; ++i)
    {
        if (nullptr == params.pTarget[i])
        {
            Clean();
            return MOS_STATUS_INVALID_PARAMETER;
        }
        VP_SURFACE *surf = m_vpInterface.GetAllocator().AllocateVpSurface(*params.pTarget[i]);
        if (nullptr == surf)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_OutputSurfaces.push_back(surf);

        // Initialize m_OutputPipes.
        SwFilterSubPipe *pipe = MOS_New(SwFilterSubPipe);
        if (nullptr == pipe)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_OutputPipes.push_back(pipe);
    }

    UpdateSwFilterPipeType();

    MOS_STATUS status = ConfigFeatures(params, featureRule);
    if (MOS_FAILED(status))
    {
        Clean();
        return status;
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::Initialize(VEBOX_SFC_PARAMS &params)
{
    Clean();

    // Initialize input surface.
    {
        if (nullptr == params.input.surface)
        {
            Clean();
            return MOS_STATUS_INVALID_PARAMETER;
        }
        // The value of plane offset are different between vp and codec. updatePlaneOffset need be set to true when create vp surface
        // with mos surface from codec hal.
        VP_SURFACE *input = m_vpInterface.GetAllocator().AllocateVpSurface(*params.input.surface, params.input.colorSpace,
                                                                        params.input.chromaSiting, params.input.rcSrc,
                                                                        params.output.rcDst, SURF_IN_PRIMARY, true);
        if (nullptr == input)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_InputSurfaces.push_back(input);
        // Keep m_PastSurface same size as m_InputSurfaces.
        m_pastSurface.push_back(nullptr);
        m_futureSurface.push_back(nullptr);

        // Initialize m_InputPipes.
        SwFilterSubPipe *pipe = MOS_New(SwFilterSubPipe);
        if (nullptr == pipe)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_InputPipes.push_back(pipe);
    }

    // Initialize output surface.
    {
        if (nullptr == params.output.surface)
        {
            Clean();
            return MOS_STATUS_INVALID_PARAMETER;
        }

        RECT recOutput = {0, 0, (int32_t)params.output.surface->dwWidth, (int32_t)params.output.surface->dwHeight};
        // The value of plane offset are different between vp and codec. updatePlaneOffset need be set to true when create vp surface
        // with mos surface from codec hal.
        VP_SURFACE *output = m_vpInterface.GetAllocator().AllocateVpSurface(*params.output.surface, params.output.colorSpace,
                                                                        params.output.chromaSiting, recOutput,
                                                                        recOutput, SURF_OUT_RENDERTARGET, true);
        if (nullptr == output)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_OutputSurfaces.push_back(output);

        // Initialize m_OutputPipes.
        SwFilterSubPipe *pipe = MOS_New(SwFilterSubPipe);
        if (nullptr == pipe)
        {
            Clean();
            return MOS_STATUS_NULL_POINTER;
        }
        m_OutputPipes.push_back(pipe);
    }

    UpdateSwFilterPipeType();

    MOS_STATUS status = ConfigFeatures(params);
    if (MOS_FAILED(status))
    {
        Clean();
        return status;
    }

    return MOS_STATUS_SUCCESS;
}

void SwFilterPipe::UpdateSwFilterPipeType()
{
    m_swFilterPipeType = SwFilterPipeTypeInvalid;

    if (1 == m_InputSurfaces.size() && 1 == m_OutputSurfaces.size())
    {
        m_swFilterPipeType = SwFilterPipeType1To1;
    }
    else if (m_InputSurfaces.size() > 1 && 1 == m_OutputSurfaces.size())
    {
        m_swFilterPipeType = SwFilterPipeTypeNTo1;
    }
    else if (1 == m_InputSurfaces.size() && m_OutputSurfaces.size() > 1)
    {
        m_swFilterPipeType = SwFilterPipeType1ToN;
    }
    else if (0 == m_InputSurfaces.size() && 1 == m_OutputSurfaces.size())
    {
        m_swFilterPipeType = SwFilterPipeType0To1;
    }
}

MOS_STATUS SwFilterPipe::Clean()
{
    // Do not unregister features in Clean, which will be reused until swFilterPipe being destroyed.
    m_swFilterPipeType = SwFilterPipeTypeInvalid;

    CleanFeatures();

    std::vector<SwFilterSubPipe *> *pipes[] = {&m_InputPipes, &m_OutputPipes};
    for (auto pipe : pipes)
    {
        while (!pipe->empty())
        {
            auto p = pipe->back();
            MOS_Delete(p);
            pipe->pop_back();
        }
    }

    std::vector<VP_SURFACE *> *surfacesArray[] = {&m_InputSurfaces, &m_OutputSurfaces, &m_pastSurface, &m_futureSurface};
    for (auto surfaces : surfacesArray)
    {
        while (!surfaces->empty())
        {
            auto p = surfaces->back();
            m_vpInterface.GetAllocator().DestroyVpSurface(p);
            surfaces->pop_back();
        }
    }

    return MOS_STATUS_SUCCESS;
}

bool SwFilterPipe::IsEmpty()
{
    if (0 == m_OutputSurfaces.size())
    {
        return true;
    }
    return false;
}

bool SwFilterPipe::IsPrimaryEmpty()
{
    uint32_t index;
    auto pipe = GetSwFilterPrimaryPipe(index);

    if (pipe->IsEmpty())
    {
        return true;
    }

    return false;
}

MOS_STATUS SwFilterPipe::ConfigFeaturesToPipe(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule, bool isInputPipe)
{
    std::vector<SwFilterSubPipe *>  &pipes = isInputPipe ? m_InputPipes : m_OutputPipes;
    std::vector<FeatureSubRule>     &rulePipes = isInputPipe ? featureRule.m_InputPipes : featureRule.m_OutputPipes;
    SwFilter *swFilter = nullptr;
    // Loop all input surfaces.
    for (uint32_t pipeIndex = 0; pipeIndex < pipes.size(); ++pipeIndex)
    {
        // Get a copy list for feature handler.
        auto featureHander = *m_vpInterface.GetSwFilterHandlerMap();

        if (pipeIndex < rulePipes.size())
        {
            auto &featurePipe = rulePipes[pipeIndex];

            for (auto &featureSet : featurePipe.m_Rule)
            {
                // Loop orderred feature set.
                bool useNewSwFilterSet = true;
                for (auto &feature : featureSet.m_Features)
                {
                    // Loop all features in feature set.
                    SwFilterFeatureHandler *handler = featureHander.find(feature)->second;
                    VP_PUBLIC_CHK_NULL_RETURN(handler);
                    VP_PUBLIC_CHK_STATUS_RETURN(handler->CreateSwFilter(swFilter, params, isInputPipe, pipeIndex, m_swFilterPipeType));
                    if (swFilter)
                    {
                        VP_PUBLIC_CHK_STATUS_RETURN(AddSwFilterOrdered(swFilter, isInputPipe, pipeIndex, useNewSwFilterSet));
                        featureHander.erase(feature);
                        useNewSwFilterSet = false;
                    }
                    // nullptr == swFilter means no such feature in params, which is also the valid case.
                }
            }
        }

        // Process remaining unordered features
        for (auto &handler : featureHander)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(handler.second->CreateSwFilter(swFilter, params, isInputPipe, pipeIndex, m_swFilterPipeType));
            if (swFilter)
            {
                VP_PUBLIC_CHK_STATUS_RETURN(AddSwFilterUnordered(swFilter, isInputPipe, pipeIndex));
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::ConfigFeatures(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule)
{
    MOS_STATUS status1 = ConfigFeaturesToPipe(params, featureRule, true); 
    MOS_STATUS status2 = ConfigFeaturesToPipe(params, featureRule, false);

    // Return the status for the failure one.
    return MOS_STATUS_SUCCESS == status1 ? status2 : status1;
}

MOS_STATUS SwFilterPipe::ConfigFeatures(VEBOX_SFC_PARAMS &params)
{
    std::vector<SwFilterSubPipe *>  &pipes = m_InputPipes;

    SwFilter *swFilter = nullptr;
    // Loop all input surfaces.
    for (uint32_t pipeIndex = 0; pipeIndex < pipes.size(); ++pipeIndex)
    {
        auto featureHander = m_vpInterface.GetSwFilterHandlerMap();
        // Process remaining unordered features
        if (featureHander)
        {
            for (auto handler = featureHander->begin(); handler != featureHander->end(); handler++)
            {
                VP_PUBLIC_CHK_STATUS_RETURN(handler->second->CreateSwFilter(swFilter, params));
                if (swFilter)
                {
                    VP_PUBLIC_CHK_STATUS_RETURN(AddSwFilterUnordered(swFilter, true, 0));
                }
            }
        }
        else
        {
            VP_PUBLIC_ASSERTMESSAGE("No VP Feature Manager Created");
            return MOS_STATUS_UNIMPLEMENTED;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::UpdateFeatures(bool isInputPipe, uint32_t pipeIndex)
{
    auto &pipes = isInputPipe ? m_InputPipes : m_OutputPipes;
    auto &surfaces = isInputPipe ? m_InputSurfaces : m_OutputSurfaces;

    if (pipeIndex >= pipes.size() || pipeIndex >= surfaces.size() || 0 == m_OutputPipes.size() ||
        m_InputPipes.size() != m_InputSurfaces.size() || m_OutputPipes.size() != m_OutputSurfaces.size())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Always use index 0 for the pipe whose pipeIndex not being specified.
    auto inputPipe = isInputPipe ? m_InputPipes[pipeIndex] : (m_InputPipes.size() > 0 ? m_InputPipes[0] : nullptr);
    auto outputPipe = isInputPipe ? m_OutputPipes[0] : m_OutputPipes[pipeIndex];

    auto inputSurf = isInputPipe ? m_InputSurfaces[pipeIndex] : (m_InputSurfaces.size() > 0 ? m_InputSurfaces[0] : nullptr);
    auto outputSurf = isInputPipe ? m_OutputSurfaces[0] : m_OutputSurfaces[pipeIndex];

    if (nullptr == outputPipe || nullptr == outputSurf)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    // Input surface/pipe may be empty for some feature.
    if (inputPipe)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(inputPipe->Update(inputSurf, outputSurf));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(outputPipe->Update(inputSurf, outputSurf));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::CleanFeaturesFromPipe(bool isInputPipe, uint32_t index)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    std::vector<SwFilterSubPipe *>  &pipes = isInputPipe ? m_InputPipes : m_OutputPipes;

    if (index < pipes.size() && pipes[index])
    {
        return pipes[index]->Clean();
    }

    return MOS_STATUS_INVALID_PARAMETER;
}

MOS_STATUS SwFilterPipe::CleanFeaturesFromPipe(bool isInputPipe)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    std::vector<SwFilterSubPipe *>  &pipes = isInputPipe ? m_InputPipes : m_OutputPipes;
    for (uint32_t pipeIndex = 0; pipeIndex < pipes.size(); ++pipeIndex)
    {
        MOS_STATUS tmpStat = CleanFeaturesFromPipe(isInputPipe, pipeIndex);
        if (MOS_FAILED(tmpStat))
        {
            VP_PUBLIC_ASSERTMESSAGE("CleanFeaturesFromPipe failed for pipe %d with status = %08x.", pipeIndex, tmpStat);
            // Always record first failure.
            status = MOS_FAILED(status) ? status : tmpStat;
        }
    }

    // Do not clear pipe here since it should align with m_InputSurfaces/m_OutputSurfaces.

    return status;
}

MOS_STATUS SwFilterPipe::CleanFeatures()
{
    MOS_STATUS status1 = CleanFeaturesFromPipe(true);
    MOS_STATUS status2 = CleanFeaturesFromPipe(false);

    // Return the status for the failure one.
    return MOS_STATUS_SUCCESS == status1 ? status2 : status1;
}

SwFilter *SwFilterPipe::GetSwFilter(bool isInputPipe, int index, FeatureType type)
{
    SwFilterSubPipe * pipe = GetSwFilterSubPipe(isInputPipe, index);
    if (nullptr == pipe)
    {
        VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! No sub pipe exists!");
        return nullptr;
    }

    return pipe->GetSwFilter(type);
}

SwFilterSubPipe* SwFilterPipe::GetSwFilterSubPipe(bool isInputPipe, int index)
{
    SwFilterSubPipe* pSubPipe = nullptr;
    auto& pipes = isInputPipe ? m_InputPipes : m_OutputPipes;

    if ((uint32_t)index < pipes.size())
    {
        pSubPipe = pipes[index];
    }
    return pSubPipe;
}

SwFilterSubPipe* SwFilterPipe::GetSwFilterPrimaryPipe(uint32_t& index)
{
    SwFilterSubPipe* pSubPipe = nullptr;

    index = 0;

    if (m_InputPipes.size() == 0)
    {
        return nullptr;
    }

    for (auto item : m_InputSurfaces)
    {

        if (item->SurfType == SURF_IN_PRIMARY)
        {
            pSubPipe = m_InputPipes[index];
            return pSubPipe;
        }
        ++index;
    }
    return pSubPipe;
}

// useNewSwFilterSet: true if insert new swFilterSet in pipe, otherwise, reuse the last one in pipe.
MOS_STATUS SwFilterPipe::AddSwFilterOrdered(SwFilter *swFilter, bool isInputPipe, int index, bool useNewSwFilterSet)
{
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);

    SwFilterSubPipe *pSubPipe = GetSwFilterSubPipe(isInputPipe, index);
    VP_PUBLIC_CHK_NULL_RETURN(pSubPipe);

    return pSubPipe->AddSwFilterOrdered(swFilter, useNewSwFilterSet);
}

MOS_STATUS SwFilterPipe::AddSwFilterUnordered(SwFilter *swFilter, bool isInputPipe, int index)
{
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);

    SwFilterSubPipe *pSubPipe = GetSwFilterSubPipe(isInputPipe, index);
    VP_PUBLIC_CHK_NULL_RETURN(pSubPipe);

    return pSubPipe->AddSwFilterUnordered(swFilter);
}

MOS_STATUS SwFilterPipe::RemoveSwFilter(SwFilter *swFilter)
{
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);

    SwFilterSet *swFilterSet = swFilter->GetLocation();
    VP_PUBLIC_CHK_NULL_RETURN(swFilterSet);

    VP_PUBLIC_CHK_STATUS_RETURN(swFilterSet->RemoveSwFilter(swFilter));

    // Delete swFilterSet if needed.
    auto pipe = swFilterSet->GetLocation();
    if (pipe && swFilterSet->IsEmpty())
    {
        for (auto it = pipe->begin(); it != pipe->end(); ++it)
        {
            if (*it == swFilterSet)
            {
                pipe->erase(it);
                break;
            }
        }
        swFilterSet->SetLocation(nullptr);

        MOS_Delete(swFilterSet);
    }
    return MOS_STATUS_SUCCESS;
}

VP_SURFACE *SwFilterPipe::GetSurface(bool isInputSurface, uint32_t index)
{
    if (isInputSurface)
    {
        return index < m_InputSurfaces.size() ? m_InputSurfaces[index] : nullptr;
    }
    else
    {
        return index < m_OutputSurfaces.size() ? m_OutputSurfaces[index] : nullptr;
    }
}

VP_SURFACE *SwFilterPipe::GetPastSurface(uint32_t index)
{
    return index < m_pastSurface.size() ? m_pastSurface[index] : nullptr;
}

VP_SURFACE *SwFilterPipe::GetFutureSurface(uint32_t index)
{
    return index < m_futureSurface.size() ? m_futureSurface[index] : nullptr;
}

VP_SURFACE *SwFilterPipe::RemoveSurface(bool isInputSurface, uint32_t index)
{
    auto &surfaces = isInputSurface ? m_InputSurfaces : m_OutputSurfaces;

    if (index < surfaces.size())
    {
        VP_SURFACE *surf = surfaces[index];
        surfaces[index] = nullptr;

        if (isInputSurface)
        {
            // Keep m_pastSurface and m_futureSurface same status as m_InputSurfaces.
            m_pastSurface[index] = nullptr;
            m_futureSurface[index] = nullptr;
        }

        return surf;
    }
    return nullptr;
}

MOS_STATUS SwFilterPipe::AddSurface(VP_SURFACE *&surf, bool isInputSurface, uint32_t index)
{
    auto &surfaces = isInputSurface ? m_InputSurfaces : m_OutputSurfaces;
    auto &pipes = isInputSurface ? m_InputPipes : m_OutputPipes;

    for (uint32_t i = surfaces.size(); i <= index; ++i)
    {
        surfaces.push_back(nullptr);
        if (isInputSurface)
        {
            // Keep m_PastSurface same size as m_InputSurfaces.
            m_pastSurface.push_back(nullptr);
            m_futureSurface.push_back(nullptr);
        }
    }

    if (index >= surfaces.size())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (nullptr != surfaces[index])
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    for (uint32_t i = pipes.size(); i <= index; ++i)
    {
        pipes.push_back(nullptr);
    }

    if (index >= surfaces.size())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    if (nullptr == pipes[index])
    {
        SwFilterSubPipe *pipe = MOS_New(SwFilterSubPipe);
        VP_PUBLIC_CHK_NULL_RETURN(pipe);
        pipes[index] = pipe;
    }

    surfaces[index] = surf;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::RemoveUnusedLayers(bool bUpdateInput)
{
    // If bUpdateInput == true, surfaces1 is input layers, which will be updated, and surfaces2 is output layers,
    // otherwise, surfaces1 is output layers, which will be updated, and surfaces2 is input layers.
    auto &surfaces1 = bUpdateInput ? m_InputSurfaces : m_OutputSurfaces;
    auto &surfaces2 = !bUpdateInput ? m_InputSurfaces : m_OutputSurfaces;

    auto &pipes = bUpdateInput ? m_InputPipes : m_OutputPipes;

    std::vector<uint32_t> indexForRemove;
    uint32_t i = 0;
    for (i = 0; i < surfaces1.size(); ++i)
    {
        if (nullptr == surfaces1[i] || 0 == surfaces2.size() ||
            1 == surfaces2.size() && nullptr == surfaces2[0])
        {
            indexForRemove.push_back(i);
            CleanFeaturesFromPipe(bUpdateInput, i);
        }
    }

    for (auto index : indexForRemove)
    {
        auto itSurf = surfaces1.begin();
        for (i = 0; itSurf != surfaces1.end(); ++itSurf, ++i)
        {
            if (i == index)
            {
                surfaces1.erase(itSurf);
                break;
            }
        }

        if (bUpdateInput)
        {
            // Keep m_pastSurface same size as m_InputSurfaces.
            itSurf = m_pastSurface.begin();
            for (i = 0; itSurf != m_pastSurface.end(); ++itSurf, ++i)
            {
                if (i == index)
                {
                    m_pastSurface.erase(itSurf);
                    break;
                }
            }

            // Keep m_futureSurface same size as m_InputSurfaces.
            itSurf = m_futureSurface.begin();
            for (i = 0; itSurf != m_futureSurface.end(); ++itSurf, ++i)
            {
                if (i == index)
                {
                    m_futureSurface.erase(itSurf);
                    break;
                }
            }
        }

        auto itPipe = pipes.begin();
        for (i = 0; itPipe != pipes.end(); ++itPipe, ++i)
        {
            if (i == index)
            {
                auto pipe = *itPipe;
                pipes.erase(itPipe);
                MOS_Delete(pipe);
                break;
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::Update()
{
    uint32_t i = 0;

    VP_PUBLIC_CHK_STATUS_RETURN(RemoveUnusedLayers(true));
    VP_PUBLIC_CHK_STATUS_RETURN(RemoveUnusedLayers(false));

    for (i = 0; i < m_InputPipes.size(); ++i)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatures(true, i));
    }

    for (i = 0; i < m_OutputPipes.size(); ++i)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatures(false, i));
    }

    UpdateSwFilterPipeType();

    return MOS_STATUS_SUCCESS;
}

uint32_t SwFilterPipe::GetSurfaceCount(bool isInputSurface)
{
    return isInputSurface ? m_InputSurfaces.size() : m_OutputSurfaces.size();
}