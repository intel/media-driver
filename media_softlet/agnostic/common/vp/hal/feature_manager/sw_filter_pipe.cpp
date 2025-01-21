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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    for (auto &featureSet : m_OrderedFilters)
    {
        if (featureSet)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(featureSet->Update(inputSurf, outputSurf, *this));
        }
    }
    VP_PUBLIC_CHK_STATUS_RETURN(m_UnorderedFilters.Update(inputSurf, outputSurf, *this));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSubPipe::AddFeatureGraphRTLog()
{
    VP_FUNC_CALL();

    for (auto &featureSet : m_OrderedFilters)
    {
        if (featureSet)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(featureSet->AddFeatureGraphRTLog());
        }
    }
    VP_PUBLIC_CHK_STATUS_RETURN(m_UnorderedFilters.AddFeatureGraphRTLog());

    return MOS_STATUS_SUCCESS;
}

SwFilter *SwFilterSubPipe::GetSwFilter(FeatureType type)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(swFilter);
    return m_UnorderedFilters.AddSwFilter(swFilter);
}

MOS_STATUS SwFilterSubPipe::GetAiSwFilter(SwFilterAiBase *&swAiFilter)
{
    SwFilterAiBase *aiFilter = nullptr;
    swAiFilter               = nullptr;
    for (SwFilterSet*& swFilterSet : m_OrderedFilters)
    {
        if (swFilterSet)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(swFilterSet->GetAiSwFilter(aiFilter));
            if (aiFilter)
            {
                if (swAiFilter)
                {
                    VP_PUBLIC_ASSERTMESSAGE("Only one AI Sw Filter is allowed in one SwFilterSubPipe. More than one is found. The last one is in ordered filters. Feature Types: %d and %d", swAiFilter->GetFeatureType(), aiFilter->GetFeatureType());
                    VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
                }
                swAiFilter = aiFilter;
            }
        }
    }
    VP_PUBLIC_CHK_STATUS_RETURN(m_UnorderedFilters.GetAiSwFilter(aiFilter));
    if (aiFilter)
    {
        if (swAiFilter)
        {
            VP_PUBLIC_ASSERTMESSAGE("Only one AI Sw Filter is allowed in one SwFilterSubPipe. More than one is found. The last one is in unordered filters. Feature Types: %d and %d", swAiFilter->GetFeatureType(), aiFilter->GetFeatureType());
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        swAiFilter = aiFilter;
    }
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterPipe                                                */
/****************************************************************************************************/

SwFilterPipe::SwFilterPipe(VpInterface &vpInterface) : m_vpInterface(vpInterface)
{
    m_surfacesSetting.Clean();
}

SwFilterPipe::~SwFilterPipe()
{
    Clean();
}

MOS_STATUS SwFilterPipe::Initialize(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule)
{
    VP_FUNC_CALL();

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
            MT_ERR2(MT_VP_HAL_SWWFILTER, MT_CODE_LINE, __LINE__, MT_ERROR_CODE, MOS_STATUS_NULL_POINTER);
            return MOS_STATUS_NULL_POINTER;
        }

        surf->Palette = params.pSrc[i]->Palette;

        m_InputSurfaces.push_back(surf);

        // Keep m_pastSurface/m_futureSurface same size as m_InputSurfaces.
        VP_SURFACE *pastSurface = nullptr;
        if (params.pSrc[i]->uBwdRefCount > 0 && params.pSrc[i]->pBwdRef &&
            params.pSrc[i]->FrameID != params.pSrc[i]->pBwdRef->FrameID)
        {
            pastSurface = m_vpInterface.GetAllocator().AllocateVpSurface(*params.pSrc[i]->pBwdRef);
        }
        VP_SURFACE *futureSurface = nullptr;
        if (params.pSrc[i]->uFwdRefCount > 0 && params.pSrc[i]->pFwdRef &&
            params.pSrc[i]->FrameID != params.pSrc[i]->pFwdRef->FrameID)
        {
            futureSurface = m_vpInterface.GetAllocator().AllocateVpSurface(*params.pSrc[i]->pFwdRef);
        }
        m_pastSurface.push_back(pastSurface);
        m_futureSurface.push_back(futureSurface);
        m_linkedLayerIndex.push_back(0);

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
            MT_ERR2(MT_VP_HAL_SWWFILTER, MT_CODE_LINE, __LINE__, MT_ERROR_CODE, MOS_STATUS_NULL_POINTER);
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
    VP_FUNC_CALL();

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
        m_linkedLayerIndex.push_back(0);

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    m_linkedLayerIndex.clear();

    m_isExePipe = false;

    return MOS_STATUS_SUCCESS;
}

bool SwFilterPipe::IsEmpty()
{
    VP_FUNC_CALL();

    for (auto pipe : m_InputPipes)
    {
        if (!pipe->IsEmpty())
        {
            return false;
        }
    }

    for (auto pipe : m_OutputPipes)
    {
        if (!pipe->IsEmpty())
        {
            return false;
        }
    }

    return true;
}

bool SwFilterPipe::IsPrimaryEmpty()
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
                MT_LOG4(MT_VP_HAL_SWWFILTER_ADD, MT_NORMAL, MT_VP_HAL_PIPE_ISINPUT, isInputPipe, MT_VP_HAL_PIPE_INDEX, pipeIndex, 
                    MT_VP_HAL_FEATUERTYPE, swFilter->GetFeatureType(), MT_VP_HAL_ENGINECAPS, int64_t(swFilter->GetFilterEngineCaps().value));
            }
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::ConfigFeatures(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule)
{
    VP_FUNC_CALL();

    MOS_STATUS status1 = ConfigFeaturesToPipe(params, featureRule, true); 
    MOS_STATUS status2 = ConfigFeaturesToPipe(params, featureRule, false);

    // Return the status for the failure one.
    return MOS_STATUS_SUCCESS == status1 ? status2 : status1;
}

MOS_STATUS SwFilterPipe::ConfigFeatures(VEBOX_SFC_PARAMS &params)
{
    VP_FUNC_CALL();

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

MOS_STATUS SwFilterPipe::UpdateFeatures(bool isInputPipe, uint32_t pipeIndex, VP_EXECUTE_CAPS *caps)
{
    VP_FUNC_CALL();

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
        if (caps && caps->bComposite)
        {
            // Always keep csc filter in fc pipe. The csc filter for multi-pass intermediate surface case will be added here.
            if (nullptr == inputPipe->GetSwFilter(FeatureTypeCsc))
            {
                auto handler = m_vpInterface.GetSwFilterHandler(FeatureTypeCsc);
                VP_PUBLIC_CHK_NULL_RETURN(handler);
                SwFilterCsc* swfilter = dynamic_cast<SwFilterCsc *>(handler->CreateSwFilter());
                VP_PUBLIC_CHK_NULL_RETURN(swfilter);
                swfilter->Configure(inputSurf, outputSurf, *caps);
                inputPipe->AddSwFilterUnordered(swfilter);
            }
        }

        VP_PUBLIC_CHK_STATUS_RETURN(inputPipe->Update(inputSurf, outputSurf));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(outputPipe->Update(inputSurf, outputSurf));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::CleanFeaturesFromPipe(bool isInputPipe, uint32_t index)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    MOS_STATUS status1 = CleanFeaturesFromPipe(true);
    MOS_STATUS status2 = CleanFeaturesFromPipe(false);

    // Return the status for the failure one.
    return MOS_STATUS_SUCCESS == status1 ? status2 : status1;
}

SwFilter *SwFilterPipe::GetSwFilter(bool isInputPipe, int index, FeatureType type)
{
    VP_FUNC_CALL();

    SwFilterSubPipe * pipe = GetSwFilterSubPipe(isInputPipe, index);
    if (nullptr == pipe)
    {
        // May come here for ClearView case.
        VP_PUBLIC_NORMALMESSAGE("Invalid parameter! No sub pipe exists!");
        return nullptr;
    }

    return pipe->GetSwFilter(type);
}

SwFilterSubPipe* SwFilterPipe::GetSwFilterSubPipe(bool isInputPipe, int index)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(swFilter);

    SwFilterSubPipe *pSubPipe = GetSwFilterSubPipe(isInputPipe, index);
    VP_PUBLIC_CHK_NULL_RETURN(pSubPipe);

    VP_PUBLIC_CHK_STATUS_RETURN(pSubPipe->AddSwFilterOrdered(swFilter, useNewSwFilterSet));
    swFilter->SetExePipeFlag(m_isExePipe);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::AddSwFilterUnordered(SwFilter *swFilter, bool isInputPipe, int index)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(swFilter);

    SwFilterSubPipe *pSubPipe = GetSwFilterSubPipe(isInputPipe, index);

    if (nullptr == pSubPipe && !isInputPipe)
    {
        auto& pipes = m_OutputPipes;
        SwFilterSubPipe *pipe = MOS_New(SwFilterSubPipe);
        VP_PUBLIC_CHK_NULL_RETURN(pipe);
        if ((size_t)index <= pipes.size())
        {
            for (int32_t i = (int)pipes.size(); i <= index; ++i)
            {
                pipes.push_back(nullptr);
            }
        }
        pipes[index] = pipe;
        pSubPipe = GetSwFilterSubPipe(isInputPipe, index);
    }

    VP_PUBLIC_CHK_NULL_RETURN(pSubPipe);

    VP_PUBLIC_CHK_STATUS_RETURN(pSubPipe->AddSwFilterUnordered(swFilter));
    swFilter->SetExePipeFlag(m_isExePipe);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::RemoveSwFilter(SwFilter *swFilter)
{
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    return index < m_pastSurface.size() ? m_pastSurface[index] : nullptr;
}

VP_SURFACE *SwFilterPipe::GetFutureSurface(uint32_t index)
{
    VP_FUNC_CALL();

    return index < m_futureSurface.size() ? m_futureSurface[index] : nullptr;
}

MOS_STATUS SwFilterPipe::SetPastSurface(uint32_t index, VP_SURFACE *surf)
{
    VP_FUNC_CALL();

    if (index >= m_pastSurface.size())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_pastSurface[index] = surf;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::SetFutureSurface(uint32_t index, VP_SURFACE *surf)
{
    VP_FUNC_CALL();

    if (index >= m_futureSurface.size())
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_futureSurface[index] = surf;
    return MOS_STATUS_SUCCESS;
}

VP_SURFACE *SwFilterPipe::RemovePastSurface(uint32_t index)
{
    VP_FUNC_CALL();

    if (index >= m_pastSurface.size())
    {
        return nullptr;
    }
    VP_SURFACE *surf = m_pastSurface[index];
    m_pastSurface[index] = nullptr;
    return surf;
}

VP_SURFACE *SwFilterPipe::RemoveFutureSurface(uint32_t index)
{
    VP_FUNC_CALL();

    if (index >= m_futureSurface.size())
    {
        return nullptr;
    }
    VP_SURFACE *surf = m_futureSurface[index];
    m_futureSurface[index] = nullptr;
    return surf;
}

MOS_STATUS SwFilterPipe::DestroySurface(bool isInputSurface, uint32_t index)
{
    VP_SURFACE *surf = SwFilterPipe::RemoveSurface(isInputSurface, index);
    VP_PUBLIC_CHK_NULL_RETURN(surf);
    m_vpInterface.GetAllocator().DestroyVpSurface(surf);
    return MOS_STATUS_SUCCESS;
}

VP_SURFACE *SwFilterPipe::RemoveSurface(bool isInputSurface, uint32_t index)
{
    VP_FUNC_CALL();

    auto &surfaces = isInputSurface ? m_InputSurfaces : m_OutputSurfaces;

    if (index < surfaces.size())
    {
        VP_SURFACE *surf = surfaces[index];
        surfaces[index] = nullptr;

        if (isInputSurface)
        {
            // Keep m_pastSurface and m_futureSurface same status as m_InputSurfaces.
            if (m_pastSurface[index])
            {
                m_vpInterface.GetAllocator().DestroyVpSurface(m_pastSurface[index]);
            }
            if (m_futureSurface[index])
            {
                m_vpInterface.GetAllocator().DestroyVpSurface(m_futureSurface[index]);
            }
            if (m_linkedLayerIndex[index])
            {
                m_linkedLayerIndex[index] = 0;
            }
        }

        return surf;
    }
    return nullptr;
}

VP_SURFACE *SwFilterPipe::ReplaceSurface(VP_SURFACE *surf, bool isInputSurface, uint32_t index)
{
    VP_FUNC_CALL();

    auto &surfaces = isInputSurface ? m_InputSurfaces : m_OutputSurfaces;

    if (index < surfaces.size())
    {
        VP_SURFACE *ret = surfaces[index];
        surfaces[index] = surf;
        return ret;
    }
    return nullptr;
}

MOS_STATUS SwFilterPipe::AddSurface(VP_SURFACE *&surf, bool isInputSurface, uint32_t index)
{
    VP_FUNC_CALL();

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
            m_linkedLayerIndex.push_back(0);
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

template<class T>
MOS_STATUS RemoveUnusedLayers(std::vector<uint32_t> &indexForRemove, std::vector<T> &layers)
{
    VP_FUNC_CALL();

    if (indexForRemove.size() == 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (sizeof(T) < sizeof(int))
    {
        VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
    }
    const int keyForDelete = 0xabcdabcd;
    for (uint32_t index : indexForRemove)
    {
        if (index >= layers.size())
        {
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
        }
        // Mark the items to be removed.
        *(int *)&layers[index] = keyForDelete;
    }

    for (auto it = layers.begin(); it != layers.end();)
    {
        if (keyForDelete == *(int *)&(*it))
        {
            it = layers.erase(it);
        }
        else
        {
            ++it;
        }
    }

    return MOS_STATUS_SUCCESS;
}

template<class T>
MOS_STATUS RemoveUnusedLayers(std::vector<uint32_t> &indexForRemove, std::vector<T*> &layers, bool freeObj)
{
    VP_FUNC_CALL();

    if (indexForRemove.size() == 0)
    {
        return MOS_STATUS_SUCCESS;
    }

    if (freeObj)
    {
        std::map<T*, T*> objForRemove;
        for (uint32_t index : indexForRemove)
        {
            if (index >= layers.size())
            {
                VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_INVALID_PARAMETER);
            }
            objForRemove.insert(std::make_pair(layers[index], layers[index]));
            layers[index] = nullptr;
        }
        for (auto it : objForRemove)
        {
            MOS_Delete(it.second);
        }
    }

    return RemoveUnusedLayers(indexForRemove, layers);
}

MOS_STATUS SwFilterPipe::RemoveUnusedLayers(bool bUpdateInput)
{
    VP_FUNC_CALL();

    // If bUpdateInput == true, surfaces1 is input layers, which will be updated, and surfaces2 is output layers,
    // otherwise, surfaces1 is output layers, which will be updated, and surfaces2 is input layers.
    auto &surfaces1 = bUpdateInput ? m_InputSurfaces : m_OutputSurfaces;
    auto &surfaces2 = !bUpdateInput ? m_InputSurfaces : m_OutputSurfaces;
    auto &pipes = bUpdateInput ? m_InputPipes : m_OutputPipes;

    std::vector<uint32_t> indexForRemove;
    uint32_t i = 0;
    bool isInputNullValid = (!bUpdateInput && pipes.size()>0 && !pipes[0]->IsEmpty());

    for (i = 0; i < surfaces1.size(); ++i)
    {
        if (nullptr == surfaces1[i] || (!isInputNullValid && 0 == surfaces2.size()) ||
            1 == surfaces2.size() && nullptr == surfaces2[0])
        {
            indexForRemove.push_back(i);
            CleanFeaturesFromPipe(bUpdateInput, i);
        }
    }

    VP_PUBLIC_CHK_STATUS_RETURN(::RemoveUnusedLayers(indexForRemove, surfaces1, false));
    if (bUpdateInput)
    {
        // Keep m_pastSurface same size as m_InputSurfaces.
        VP_PUBLIC_CHK_STATUS_RETURN(::RemoveUnusedLayers(indexForRemove, m_pastSurface, false));
        // Keep m_futureSurface same size as m_InputSurfaces.
        VP_PUBLIC_CHK_STATUS_RETURN(::RemoveUnusedLayers(indexForRemove, m_futureSurface, false));
        // Keep m_linkedLayerIndex same size as m_InputSurfaces.
        VP_PUBLIC_CHK_STATUS_RETURN(::RemoveUnusedLayers(indexForRemove, m_linkedLayerIndex));
    }

    VP_PUBLIC_CHK_STATUS_RETURN(::RemoveUnusedLayers(indexForRemove, pipes, true));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::Update(VP_EXECUTE_CAPS *caps)
{
    VP_FUNC_CALL();

    uint32_t i = 0;

    VP_PUBLIC_CHK_STATUS_RETURN(RemoveUnusedLayers(true));
    VP_PUBLIC_CHK_STATUS_RETURN(RemoveUnusedLayers(false));

    for (i = 0; i < m_InputPipes.size(); ++i)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(UpdateFeatures(true, i, caps));
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
    VP_FUNC_CALL();

    return isInputSurface ? m_InputSurfaces.size() : m_OutputSurfaces.size();
}

bool SwFilterPipe::IsAllInputPipeEmpty()
{
    for (uint32_t i = 0; i < GetSurfaceCount(true); ++i)
    {
        SwFilterSubPipe *inputPipe = GetSwFilterSubPipe(true, i);
        if (inputPipe && !inputPipe->IsEmpty())
        {
            return false;
        }
    }

    return true;
}

bool SwFilterPipe::IsAllInputPipeSurfaceFeatureEmpty()
{
    for (uint32_t i = 0; i < GetSurfaceCount(true); ++i)
    {
        SwFilterSubPipe *inputPipe = GetSwFilterSubPipe(true, i);
        if (inputPipe && !inputPipe->IsSurfaceFeatureEmpty())
        {
            return false;
        }
    }

    return true;
}

bool SwFilterPipe::IsAllInputPipeSurfaceFeatureEmpty(std::vector<int> &layerIndexes)
{
    for (uint32_t i = 0; i < layerIndexes.size(); ++i)
    {
        uint32_t executedIndex = layerIndexes[i];
        SwFilterSubPipe *inputPipe = GetSwFilterSubPipe(true, executedIndex);
        if (inputPipe && !inputPipe->IsSurfaceFeatureEmpty())
        {
            return false;
        }
    }

    return true;
}

MOS_STATUS SwFilterPipe::QuerySwAiFilter(bool &containsSwAiFilter)
{
    containsSwAiFilter = false;
    for (SwFilterSubPipe*& inputPipe : m_InputPipes)
    {
        SwFilterAiBase* swAiFilter = nullptr;
        if (inputPipe)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(inputPipe->GetAiSwFilter(swAiFilter));
            if (swAiFilter)
            {
                containsSwAiFilter = true;
                return MOS_STATUS_SUCCESS;
            }
        }
    }
    for (SwFilterSubPipe *&outputPipe : m_OutputPipes)
    {
        SwFilterAiBase *swAiFilter = nullptr;
        if (outputPipe)
        {
            VP_PUBLIC_CHK_STATUS_RETURN(outputPipe->GetAiSwFilter(swAiFilter));
            if (swAiFilter)
            {
                containsSwAiFilter = true;
                return MOS_STATUS_SUCCESS;
            }
        }
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::AddRTLog()
{
    VP_FUNC_CALL();

    uint32_t i = 0;
    MT_LOG1(MT_VP_FEATURE_GRAPH_GET_RENDERTARGETTYPE, MT_NORMAL, MT_VP_FEATURE_GRAPH_RENDERTARGETTYPE, GetRenderTargetType())
    MT_LOG1(MT_VP_FEATURE_GRAPH_INPUTSWFILTER, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_SWFILTERPIPE_COUNT, (int64_t)m_InputPipes.size());
    for (i = 0; i < m_InputPipes.size(); ++i)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AddFeatureGraphRTLog(true, i));
    }
    MT_LOG1(MT_VP_FEATURE_GRAPH_OUTPUTSWFILTER, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_SWFILTERPIPE_COUNT, (int64_t)m_OutputPipes.size());
    for (i = 0; i < m_OutputPipes.size(); ++i)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(AddFeatureGraphRTLog(false, i));
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipe::AddFeatureGraphRTLog(bool isInputPipe, uint32_t pipeIndex)
{
    VP_FUNC_CALL();

    // Always use index 0 for the pipe whose pipeIndex not being specified.
    auto inputPipe  = m_InputPipes.size() > 0 ? (isInputPipe ? m_InputPipes[pipeIndex] : m_InputPipes[0]) : nullptr;
    auto outputPipe = m_OutputPipes.size() > 0 ? (isInputPipe ? m_OutputPipes[0] : m_OutputPipes[pipeIndex]) : nullptr;

    // Input surface/pipe may be empty for some feature.
    if (isInputPipe)
    {
        MT_LOG7(MT_VP_FEATURE_GRAPH_INPUT_SURFACE_INFO, MT_NORMAL, MT_VP_FEATURE_GRAPH_SURFACE_WIDTH, m_InputSurfaces[pipeIndex]->osSurface->dwWidth, MT_VP_FEATURE_GRAPH_SURFACE_HEIGHT, m_InputSurfaces[pipeIndex]->osSurface->dwHeight, MT_VP_FEATURE_GRAPH_SURFACE_PITCH, m_InputSurfaces[pipeIndex]->osSurface->dwPitch, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_LEFT, m_InputSurfaces[pipeIndex]->rcSrc.left, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_TOP, m_InputSurfaces[pipeIndex]->rcSrc.top, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_RIGHT, m_InputSurfaces[pipeIndex]->rcSrc.right, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_BOTTOM, m_InputSurfaces[pipeIndex]->rcSrc.bottom);
        MT_LOG7(MT_VP_FEATURE_GRAPH_INPUT_SURFACE_INFO, MT_NORMAL, MT_VP_FEATURE_GRAPH_SURFACE_COLORSPACE, m_InputSurfaces[pipeIndex]->ColorSpace, MT_VP_FEATURE_GRAPH_SURFACE_FORMAT, m_InputSurfaces[pipeIndex]->osSurface->Format, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_LEFT, m_InputSurfaces[pipeIndex]->rcDst.left, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_TOP, m_InputSurfaces[pipeIndex]->rcDst.top, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_RIGHT, m_InputSurfaces[pipeIndex]->rcDst.right, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_BOTTOM, m_InputSurfaces[pipeIndex]->rcDst.bottom, MT_VP_FEATURE_GRAPH_SURFACE_ALLOCATIONHANDLE, static_cast<int64_t>(m_InputSurfaces[pipeIndex]->GetAllocationHandle(m_vpInterface.GetHwInterface()->m_osInterface)));
        VP_PUBLIC_NORMALMESSAGE(
            "Feature Graph: Input Surface: dwWidth %d, dwHeight %d, dwPitch %d, ColorSpace %d, Format %d, \
            rcSrc.left %d, rcSrc.top %d, rcSrc.right %d, rcSrc.bottom %d, \
            rcDst.left %d, rcDst.top %d, rcDst.right %d, rcDst.bottom %d, surface allocationhandle %d",
            m_InputSurfaces[pipeIndex]->osSurface->dwWidth,
            m_InputSurfaces[pipeIndex]->osSurface->dwHeight,
            m_InputSurfaces[pipeIndex]->osSurface->dwPitch,
            m_InputSurfaces[pipeIndex]->ColorSpace,
            m_InputSurfaces[pipeIndex]->osSurface->Format,
            m_InputSurfaces[pipeIndex]->rcSrc.left,
            m_InputSurfaces[pipeIndex]->rcSrc.top,
            m_InputSurfaces[pipeIndex]->rcSrc.right,
            m_InputSurfaces[pipeIndex]->rcSrc.bottom,
            m_InputSurfaces[pipeIndex]->rcDst.left,
            m_InputSurfaces[pipeIndex]->rcDst.top,
            m_InputSurfaces[pipeIndex]->rcDst.right,
            m_InputSurfaces[pipeIndex]->rcDst.bottom,
            m_InputSurfaces[pipeIndex]->GetAllocationHandle(m_vpInterface.GetHwInterface()->m_osInterface));
        VP_PUBLIC_CHK_STATUS_RETURN(inputPipe->AddFeatureGraphRTLog());
    }
    else
    {
        MT_LOG7(MT_VP_FEATURE_GRAPH_OUTPUT_SURFACE_INFO, MT_NORMAL, MT_VP_FEATURE_GRAPH_SURFACE_WIDTH, m_OutputSurfaces[pipeIndex]->osSurface->dwWidth, MT_VP_FEATURE_GRAPH_SURFACE_HEIGHT, m_OutputSurfaces[pipeIndex]->osSurface->dwHeight, MT_VP_FEATURE_GRAPH_SURFACE_PITCH, m_OutputSurfaces[pipeIndex]->osSurface->dwPitch, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_LEFT, m_OutputSurfaces[pipeIndex]->rcSrc.left, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_TOP, m_OutputSurfaces[pipeIndex]->rcSrc.top, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_RIGHT, m_OutputSurfaces[pipeIndex]->rcSrc.right, MT_VP_FEATURE_GRAPH_SURFACE_RCSRC_BOTTOM, m_OutputSurfaces[pipeIndex]->rcSrc.bottom);
        MT_LOG7(MT_VP_FEATURE_GRAPH_OUTPUT_SURFACE_INFO, MT_NORMAL, MT_VP_FEATURE_GRAPH_SURFACE_COLORSPACE, m_OutputSurfaces[pipeIndex]->ColorSpace, MT_VP_FEATURE_GRAPH_SURFACE_FORMAT, m_OutputSurfaces[pipeIndex]->osSurface->Format, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_LEFT, m_OutputSurfaces[pipeIndex]->rcDst.left, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_TOP, m_OutputSurfaces[pipeIndex]->rcDst.top, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_RIGHT, m_OutputSurfaces[pipeIndex]->rcDst.right, MT_VP_FEATURE_GRAPH_SURFACE_RCDST_BOTTOM, m_OutputSurfaces[pipeIndex]->rcDst.bottom, MT_VP_FEATURE_GRAPH_SURFACE_ALLOCATIONHANDLE, static_cast<int64_t>(m_OutputSurfaces[pipeIndex]->GetAllocationHandle(m_vpInterface.GetHwInterface()->m_osInterface)));
        VP_PUBLIC_NORMALMESSAGE(
            "Feature Graph: Output Surface: dwWidth %d, dwHeight %d, dwPitch %d, ColorSpace %d, Format %d, \
            rcSrc.left %d, rcSrc.top %d, rcSrc.right %d, rcSrc.bottom %d, \
            rcDst.left %d, rcDst.top %d, rcDst.right %d, rcDst.bottom %d, surface allocationhandle %d",
            m_OutputSurfaces[pipeIndex]->osSurface->dwWidth,
            m_OutputSurfaces[pipeIndex]->osSurface->dwHeight,
            m_OutputSurfaces[pipeIndex]->osSurface->dwPitch,
            m_OutputSurfaces[pipeIndex]->ColorSpace,
            m_OutputSurfaces[pipeIndex]->osSurface->Format,
            m_OutputSurfaces[pipeIndex]->rcSrc.left,
            m_OutputSurfaces[pipeIndex]->rcSrc.top,
            m_OutputSurfaces[pipeIndex]->rcSrc.right,
            m_OutputSurfaces[pipeIndex]->rcSrc.bottom,
            m_OutputSurfaces[pipeIndex]->rcDst.left,
            m_OutputSurfaces[pipeIndex]->rcDst.top,
            m_OutputSurfaces[pipeIndex]->rcDst.right,
            m_OutputSurfaces[pipeIndex]->rcDst.bottom,
            m_OutputSurfaces[pipeIndex]->GetAllocationHandle(m_vpInterface.GetHwInterface()->m_osInterface));
        VP_PUBLIC_CHK_STATUS_RETURN(outputPipe->AddFeatureGraphRTLog());
    }

    return MOS_STATUS_SUCCESS;
}
