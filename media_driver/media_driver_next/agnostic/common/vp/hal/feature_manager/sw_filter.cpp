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
//! \file     sw_filter.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "sw_filter.h"
#include "vp_obj_factories.h"
using namespace vp;

/****************************************************************************************************/
/*                                      SwFilter                                                    */
/****************************************************************************************************/

SwFilter::SwFilter(VpInterface &vpInterface, FeatureType type) : m_vpInterface(vpInterface), m_type(type)
{
}

SwFilter::~SwFilter()
{
    Clean();
    RemoveFromPipe();
}

MOS_STATUS SwFilter::RemoveFromPipe()
{
    if (m_location)
    {
        m_location->RemoveSwFilter(this);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilter::SetFeatureType(FeatureType type)
{
    if ((type & FEATURE_TYPE_MASK) != (m_type & FEATURE_TYPE_MASK))
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_type = type;

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterCsc                                                 */
/****************************************************************************************************/

SwFilterCsc::SwFilterCsc(VpInterface &vpInterface) : SwFilter(vpInterface, FeatureTypeCsc)
{
    m_Params.type = m_type;
}

SwFilterCsc::~SwFilterCsc()
{
    Clean();
}

MOS_STATUS SwFilterCsc::Clean()
{
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterCsc::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    // Parameter checking should be done in SwFilterCscHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.colorSpaceInput    = surfInput->ColorSpace;
    m_Params.colorSpaceOutput   = surfOutput->ColorSpace;
    m_Params.pIEFParams         = surfInput->pIEFParams;
    m_Params.formatInput        = surfInput->Format;
    m_Params.formatOutput       = surfOutput->Format;
    m_Params.chromaSitingInput  = surfInput->ChromaSiting;
    m_Params.chromaSitingOutput = surfOutput->ChromaSiting;

    return MOS_STATUS_SUCCESS;
}

FeatureParamCsc &SwFilterCsc::GetSwFilterParams()
{
    return m_Params;
}

SwFilter *SwFilterCsc::Clone()
{
    SwFilter *p = m_vpInterface.GetSwFilterFactory().Create(m_type);
    if (nullptr == p)
    {
        return nullptr;
    }

    SwFilterCsc *swFilter = dynamic_cast<SwFilterCsc *>(p);
    if (nullptr == swFilter)
    {
        m_vpInterface.GetSwFilterFactory().Destory(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterCsc::operator == (SwFilter& swFilter)
{
    SwFilterCsc *p = dynamic_cast<SwFilterCsc *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamCsc));
}

MOS_STATUS SwFilterCsc::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);

    m_Params.colorSpaceInput    = inputSurf->ColorSpace;
    m_Params.colorSpaceOutput   = outputSurf->ColorSpace;
    m_Params.formatInput        = inputSurf->osSurface->Format;
    m_Params.formatOutput       = outputSurf->osSurface->Format;
    m_Params.chromaSitingInput  = inputSurf->ChromaSiting;
    m_Params.chromaSitingOutput = outputSurf->ChromaSiting;

    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterScaling                                             */
/****************************************************************************************************/

SwFilterScaling::SwFilterScaling(VpInterface &vpInterface) : SwFilter(vpInterface, FeatureTypeScaling)
{
    m_Params.type = m_type;
}

SwFilterScaling::~SwFilterScaling()
{
    Clean();
}

MOS_STATUS SwFilterScaling::Clean()
{
    VP_PUBLIC_CHK_STATUS_RETURN(SwFilter::Clean());
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterScaling::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    // Parameter checking should be done in SwFilterScalingHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.scalingMode            = surfInput->ScalingMode;
    m_Params.bDirectionalScalar     = surfInput->bDirectionalScalar;
    m_Params.formatInput            = surfInput->Format;
    m_Params.rcSrcInput             = surfInput->rcSrc;

    m_Params.rcMaxSrcInput          = surfInput->rcMaxSrc;
    m_Params.dwWidthInput           = surfInput->dwWidth;
    m_Params.dwHeightInput          = surfInput->dwHeight;
    m_Params.formatOutput           = surfOutput->Format;
    m_Params.colorSpaceOutput       = surfOutput->ColorSpace;
    m_Params.pColorFillParams       = params.pColorFillParams;
    m_Params.pCompAlpha             = params.pColorFillParams ? params.pCompAlpha : nullptr;

    if (surfInput->Rotation == VPHAL_ROTATION_IDENTITY ||
        surfInput->Rotation == VPHAL_ROTATION_180 ||
        surfInput->Rotation == VPHAL_MIRROR_HORIZONTAL ||
        surfInput->Rotation == VPHAL_MIRROR_VERTICAL)
    {
        m_Params.dwWidthOutput  = surfOutput->dwWidth;
        m_Params.dwHeightOutput = surfOutput->dwHeight;

        m_Params.rcDstInput     = surfInput->rcDst;
        m_Params.rcSrcOutput    = surfOutput->rcSrc;
        m_Params.rcDstOutput    = surfOutput->rcDst;
        m_Params.rcMaxSrcOutput = surfOutput->rcMaxSrc;
    }
    else
    {
        m_Params.dwWidthOutput      = surfOutput->dwHeight;
        m_Params.dwHeightOutput     = surfOutput->dwWidth;

        RECT_ROTATE(m_Params.rcDstInput, surfInput->rcDst);
        RECT_ROTATE(m_Params.rcSrcOutput, surfOutput->rcSrc);
        RECT_ROTATE(m_Params.rcDstOutput, surfOutput->rcDst);
        RECT_ROTATE(m_Params.rcMaxSrcOutput, surfOutput->rcMaxSrc);
    }

    return MOS_STATUS_SUCCESS;
}

FeatureParamScaling &SwFilterScaling::GetSwFilterParams()
{
    return m_Params;
}

SwFilter *SwFilterScaling::Clone()
{
    SwFilter *p = m_vpInterface.GetSwFilterFactory().Create(m_type);
    if (nullptr == p)
    {
        return nullptr;
    }

    SwFilterScaling *swFilter = dynamic_cast<SwFilterScaling *>(p);
    if (nullptr == swFilter)
    {
        m_vpInterface.GetSwFilterFactory().Destory(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterScaling::operator == (SwFilter& swFilter)
{
    SwFilterScaling *p = dynamic_cast<SwFilterScaling *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamScaling));
}

MOS_STATUS SwFilterScaling::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf);
    VP_PUBLIC_CHK_NULL_RETURN(inputSurf->osSurface);
    VP_PUBLIC_CHK_NULL_RETURN(outputSurf->osSurface);

    m_Params.colorSpaceOutput   = outputSurf->ColorSpace;

    return MOS_STATUS_SUCCESS;
}
/****************************************************************************************************/
/*                                      SwFilter Rotation/Mirror                                    */
/****************************************************************************************************/

SwFilterRotMir::SwFilterRotMir(VpInterface &vpInterface) : SwFilter(vpInterface, FeatureTypeRotMir)
{
    m_Params.type = m_type;
}

SwFilterRotMir::~SwFilterRotMir()
{
    Clean();
}

MOS_STATUS SwFilterRotMir::Clean()
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterRotMir::Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex)
{
    // Parameter checking should be done in SwFilterScalingHandler::IsFeatureEnabled.
    PVPHAL_SURFACE surfInput = isInputSurf ? params.pSrc[surfIndex] : params.pSrc[0];
    PVPHAL_SURFACE surfOutput = isInputSurf ? params.pTarget[0] : params.pTarget[surfIndex];

    m_Params.rotation     = surfInput->Rotation;
    m_Params.formatInput  = surfInput->Format;
    m_Params.formatOutput = surfOutput->Format;
    return MOS_STATUS_SUCCESS;
}

FeatureParamRotMir &SwFilterRotMir::GetSwFilterParams()
{
    return m_Params;
}

SwFilter *SwFilterRotMir::Clone()
{
    SwFilter *p = m_vpInterface.GetSwFilterFactory().Create(m_type);
    if (nullptr == p)
    {
        return nullptr;
    }

    SwFilterRotMir *swFilter = dynamic_cast<SwFilterRotMir *>(p);
    if (nullptr == swFilter)
    {
        m_vpInterface.GetSwFilterFactory().Destory(p);
        return nullptr;
    }

    swFilter->m_Params = m_Params;
    return p;
}

bool SwFilterRotMir::operator == (SwFilter& swFilter)
{
    SwFilterRotMir *p = dynamic_cast<SwFilterRotMir *>(&swFilter);
    return nullptr != p && 0 == memcmp(&this->m_Params, &p->m_Params, sizeof(FeatureParamRotMir));
}

MOS_STATUS SwFilterRotMir::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    m_Params.formatInput = inputSurf->osSurface->Format;
    m_Params.formatOutput = outputSurf->osSurface->Format;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      SwFilterSet                                                 */
/****************************************************************************************************/

SwFilterSet::SwFilterSet()
{}
SwFilterSet::~SwFilterSet()
{
    Clean();
}

MOS_STATUS SwFilterSet::AddSwFilter(SwFilter *swFilter)
{
    auto it = m_swFilters.find(swFilter->GetFeatureType());
    if (m_swFilters.end() != it)
    {
        VP_PUBLIC_ASSERTMESSAGE("Invalid parameter! SwFilter for feature %d has already been exists in swFilterSet!", swFilter->GetFeatureType());
        return MOS_STATUS_INVALID_PARAMETER;
    }
    m_swFilters.insert(std::make_pair(swFilter->GetFeatureType(), swFilter));
    swFilter->SetLocation(this);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSet::RemoveSwFilter(SwFilter *swFilter)
{
    auto it = m_swFilters.find(swFilter->GetFeatureType());
    if (m_swFilters.end() == it)
    {
        // The feature does not exist in current swFilterSet.
        return MOS_STATUS_SUCCESS;
    }

    if (it->second != swFilter)
    {
        // swFilter does not belong to current swFilterSet.
        return MOS_STATUS_SUCCESS;
    }

    m_swFilters.erase(it);
    swFilter->SetLocation(nullptr);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterSet::Clean()
{
    while (!m_swFilters.empty())
    {
        auto it = m_swFilters.begin();
        auto swFilter = (*it).second;
        m_swFilters.erase(it);
        if (swFilter)
        {
            VpInterface &vpIntf = swFilter->GetVpInterface();
            vpIntf.GetSwFilterFactory().Destory(swFilter);
        }
    }
    return MOS_STATUS_SUCCESS;
}

SwFilter *SwFilterSet::GetSwFilter(FeatureType type)
{
    auto it = m_swFilters.find(type);
    if (m_swFilters.end() == it)
    {
        // The feature does not exist in current swFilterSet.
        return nullptr;
    }

    return it->second;
}

std::vector<SwFilterSet *> *SwFilterSet::GetLocation()
{
    return m_location;
}
void SwFilterSet::SetLocation(std::vector<SwFilterSet *> *location)
{
    m_location = location;
}

MOS_STATUS SwFilterSet::Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf)
{
    for (auto swFilter : m_swFilters)
    {
        VP_PUBLIC_CHK_NULL_RETURN(swFilter.second);
        VP_PUBLIC_CHK_STATUS_RETURN(swFilter.second->Update(inputSurf, outputSurf));
    }
    return MOS_STATUS_SUCCESS;
}