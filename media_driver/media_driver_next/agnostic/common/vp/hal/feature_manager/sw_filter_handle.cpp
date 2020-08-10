/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     sw_filter_handle.cpp
//! \brief    Factories for vp sw filter handle creation.
//!

#include "sw_filter_handle.h"

using namespace vp;
/****************************************************************************************************/
/*                                      SwFilterFeatureHandler                                      */
/****************************************************************************************************/

SwFilterFeatureHandler::SwFilterFeatureHandler(VpInterface& vpInterface, FeatureType type) : m_vpInterface(vpInterface), m_type(type)
{}

SwFilterFeatureHandler::~SwFilterFeatureHandler()
{}

bool SwFilterFeatureHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (isInputSurf && (uint32_t)surfIndex >= params.uSrcCount ||
        !isInputSurf && (uint32_t)surfIndex >= params.uDstCount)
    {
        // Invalid parameters.
        VP_PUBLIC_ASSERTMESSAGE("Surface index is bigger than surface count!");
        return false;
    }
    return true;
}

MOS_STATUS SwFilterFeatureHandler::CreateSwFilter(SwFilter*& swFilter, VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    swFilter = nullptr;
    if (!IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        // nullptr == swFilter means no such feature in params, which is also the valid case.
        return MOS_STATUS_SUCCESS;
    }
    swFilter = CreateSwFilter();
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);
    MOS_STATUS status = swFilter->Configure(params, isInputSurf, surfIndex);
    if (MOS_FAILED(status))
    {
        Destory(swFilter);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }
    return MOS_STATUS_SUCCESS;
}

bool SwFilterFeatureHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return false;
}

MOS_STATUS SwFilterFeatureHandler::CreateSwFilter(SwFilter*& swFilter, VEBOX_SFC_PARAMS& params)
{
    swFilter = nullptr;
    if (!IsFeatureEnabled(params))
    {
        // nullptr == swFilter means no such feature in params, which is also the valid case.
        return MOS_STATUS_SUCCESS;
    }
    swFilter = CreateSwFilter();;
    VP_PUBLIC_CHK_NULL_RETURN(swFilter);
    MOS_STATUS status = swFilter->Configure(params);
    if (MOS_FAILED(status))
    {
        Destory(swFilter);
        VP_PUBLIC_CHK_STATUS_RETURN(status);
    }
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterCscHandler                                          */
/****************************************************************************************************/

SwFilterCscHandler::SwFilterCscHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeCsc),
    m_swFilterFactory(vpInterface)
{
}

SwFilterCscHandler::~SwFilterCscHandler()
{
}

bool SwFilterCscHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (isInputSurf && (SwFilterPipeType1To1 == pipeType || SwFilterPipeTypeNTo1 == pipeType) ||
        !isInputSurf && SwFilterPipeType1ToN == pipeType)
    {
        return true;
    }
    return false;
}

SwFilter* SwFilterCscHandler::CreateSwFilter()
{
    SwFilterCsc* swFilter = nullptr;
    swFilter = dynamic_cast<SwFilterCsc*>(m_swFilterFactory.Create());
    if (swFilter)
    {
        MOS_STATUS status = swFilter->SetFeatureType(FeatureTypeCsc);
        if (MOS_FAILED(status))
        {
            m_swFilterFactory.Destory(swFilter);
            return nullptr;
        }
        return swFilter;
    }
    return nullptr;
}

bool SwFilterCscHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return true;
}

void SwFilterCscHandler::Destory(SwFilter*& swFilter)
{
    SwFilterCsc* filter = nullptr;
    filter = dynamic_cast<SwFilterCsc*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterRotMirHandler                                       */
/****************************************************************************************************/

SwFilterRotMirHandler::SwFilterRotMirHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeRotMir),
    m_swFilterFactory(vpInterface)
{}
SwFilterRotMirHandler::~SwFilterRotMirHandler()
{}

bool SwFilterRotMirHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (isInputSurf && (SwFilterPipeType1To1 == pipeType || SwFilterPipeTypeNTo1 == pipeType) ||
        !isInputSurf && SwFilterPipeType1ToN == pipeType)
    {
        return true;
    }
    return false;
}

SwFilter* SwFilterRotMirHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeRotMir);
    }

    return swFilter;
}

bool SwFilterRotMirHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return true;
}

void SwFilterRotMirHandler::Destory(SwFilter*& swFilter)
{
    SwFilterRotMir* filter = nullptr;
    filter = dynamic_cast<SwFilterRotMir*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterScalingHandler                                      */
/****************************************************************************************************/

SwFilterScalingHandler::SwFilterScalingHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeScaling),
    m_swFilterFactory(vpInterface)
{}
SwFilterScalingHandler::~SwFilterScalingHandler()
{}

bool SwFilterScalingHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    if (isInputSurf && (SwFilterPipeType1To1 == pipeType || SwFilterPipeTypeNTo1 == pipeType) ||
        !isInputSurf && SwFilterPipeType1ToN == pipeType)
    {
        return true;
    }
    return false;
}

SwFilter* SwFilterScalingHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeScaling);
    }

    return swFilter;
}

bool SwFilterScalingHandler::IsFeatureEnabled(VEBOX_SFC_PARAMS& params)
{
    return true;
}

void SwFilterScalingHandler::Destory(SwFilter*& swFilter)
{
    SwFilterScaling* filter = nullptr;
    filter = dynamic_cast<SwFilterScaling*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterDnHandler                                      */
/****************************************************************************************************/

SwFilterDnHandler::SwFilterDnHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeDn),
    m_swFilterFactory(vpInterface)
{}
SwFilterDnHandler::~SwFilterDnHandler()
{}

bool SwFilterDnHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pDenoiseParams &&
        (vphalSurf->pDenoiseParams->bEnableLuma || vphalSurf->pDenoiseParams->bEnableHVSDenoise))
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterDnHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeDn);
    }

    return swFilter;
}

void SwFilterDnHandler::Destory(SwFilter*& swFilter)
{
    SwFilterDenoise* filter = nullptr;
    filter = dynamic_cast<SwFilterDenoise*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}

/****************************************************************************************************/
/*                                      SwFilterAceHandler                                      */
/****************************************************************************************************/

SwFilterAceHandler::SwFilterAceHandler(VpInterface& vpInterface) :
    SwFilterFeatureHandler(vpInterface, FeatureTypeAce),
    m_swFilterFactory(vpInterface)
{}
SwFilterAceHandler::~SwFilterAceHandler()
{}

bool SwFilterAceHandler::IsFeatureEnabled(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex, SwFilterPipeType pipeType)
{
    if (!SwFilterFeatureHandler::IsFeatureEnabled(params, isInputSurf, surfIndex, pipeType))
    {
        return false;
    }

    PVPHAL_SURFACE vphalSurf = isInputSurf ? params.pSrc[surfIndex] : params.pTarget[surfIndex];
    if (vphalSurf && vphalSurf->pColorPipeParams &&
        vphalSurf->pColorPipeParams->bEnableACE)
    {
        return true;
    }

    return false;
}

SwFilter* SwFilterAceHandler::CreateSwFilter()
{
    SwFilter* swFilter = nullptr;
    swFilter = m_swFilterFactory.Create();

    if (swFilter)
    {
        swFilter->SetFeatureType(FeatureTypeAce);
    }

    return swFilter;
}

void SwFilterAceHandler::Destory(SwFilter*& swFilter)
{
    SwFilterAce* filter = nullptr;
    filter = dynamic_cast<SwFilterAce*>(swFilter);
    m_swFilterFactory.Destory(filter);
    return;
}
