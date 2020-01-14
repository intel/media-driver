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
//! \file     vp_obj_factories.cpp
//! \brief    Factories for vp object creation.
//!
#include "vp_obj_factories.h"
using namespace vp;

/****************************************************************************************************/
/*                                      HwFilterPipeFactory                                         */
/****************************************************************************************************/

HwFilterPipeFactory::HwFilterPipeFactory(VpInterface &vpInterface) : m_allocator(vpInterface)
{
}

HwFilterPipeFactory::~HwFilterPipeFactory()
{
}

MOS_STATUS HwFilterPipeFactory::Create(SwFilterPipe &swfilterPipe,
    Policy &policy, HwFilterPipe *&pHwFilterPipe)
{
    pHwFilterPipe = m_allocator.Create();

    VP_PUBLIC_CHK_NULL_RETURN(pHwFilterPipe);
    MOS_STATUS status = pHwFilterPipe->Initialize(swfilterPipe, policy);
    if (MOS_FAILED(status))
    {
        Destory(pHwFilterPipe);
    }
    return status;
}

MOS_STATUS HwFilterPipeFactory::Destory(HwFilterPipe *&pHwfilterPipe)
{
    return m_allocator.Destory(pHwfilterPipe);
}

/****************************************************************************************************/
/*                                      HwFilterFactory                                             */
/****************************************************************************************************/

HwFilterFactory::HwFilterFactory(VpInterface &vpInterface) : m_allocatorVebox(vpInterface), m_allocatorSfc(vpInterface), m_allocatorRender(vpInterface)
{
}

HwFilterFactory::~HwFilterFactory()
{
}

HwFilter *HwFilterFactory::Create(HW_FILTER_PARAMS &param)
{
    HwFilter *p = nullptr;
    switch (param.Type)
    {
    case EngineTypeVebox:
        p = m_allocatorVebox.Create();
        break;
    case EngineTypeSfc:
        p = m_allocatorSfc.Create();
        break;
    case EngineTypeRender:
        p = m_allocatorRender.Create();
        break;
    default:
        return nullptr;
        break;
    }
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            Destory(p);
            return nullptr;
        }
    }
    return p;
}

void HwFilterFactory::Destory(HwFilter *&pHwFilter)
{
    if (pHwFilter)
    {
        switch (pHwFilter->GetEngineType())
        {
        case EngineTypeVebox:
        {
            HwFilterVebox *p = dynamic_cast<HwFilterVebox*>(pHwFilter);
            if (p)
            {
                m_allocatorVebox.Destory(p);
                pHwFilter = nullptr;
            }
            else
            {
                VP_PUBLIC_ASSERTMESSAGE("Invalid engine type for hwFilter!");
                MOS_Delete(pHwFilter);
            }
            break;
        }
        case EngineTypeSfc:
        {
            HwFilterSfc *p = dynamic_cast<HwFilterSfc*>(pHwFilter);
            if (p)
            {
                m_allocatorSfc.Destory(p);
                pHwFilter = nullptr;
            }
            else
            {
                VP_PUBLIC_ASSERTMESSAGE("Invalid engine type for hwFilter!");
                MOS_Delete(pHwFilter);
            }
            break;
        }
        case EngineTypeRender:
        {
            HwFilterRender *p = dynamic_cast<HwFilterRender*>(pHwFilter);
            if (p)
            {
                m_allocatorRender.Destory(p);
                pHwFilter = nullptr;
            }
            else
            {
                VP_PUBLIC_ASSERTMESSAGE("Invalid engine type for hwFilter!");
                MOS_Delete(pHwFilter);
            }
            break;
        }
        default:
            MOS_Delete(pHwFilter);
            return;
        }
        pHwFilter = nullptr;
    }
}

/****************************************************************************************************/
/*                                      SwFilterPipeFactory                                         */
/****************************************************************************************************/

SwFilterPipeFactory::SwFilterPipeFactory(VpInterface &vpInterface) : m_allocator(vpInterface)
{
}

SwFilterPipeFactory::~SwFilterPipeFactory()
{
}

MOS_STATUS SwFilterPipeFactory::Create(VP_PIPELINE_PARAMS &params, SwFilterPipe *&swFilterPipe)
{
    swFilterPipe = m_allocator.Create();
    VP_PUBLIC_CHK_NULL_RETURN(swFilterPipe);

    FeatureRule featureRule;
    MOS_STATUS status = swFilterPipe->Initialize(params, featureRule);

    if (MOS_FAILED(status))
    {
        m_allocator.Destory(swFilterPipe);
    }
    return status;
}

MOS_STATUS SwFilterPipeFactory::Create(SwFilterPipe *&swFilterPipe)
{
    swFilterPipe = m_allocator.Create();
    VP_PUBLIC_CHK_NULL_RETURN(swFilterPipe);

    return MOS_STATUS_SUCCESS;
}

void SwFilterPipeFactory::Destory(SwFilterPipe *&swFilterPipe)
{
    m_allocator.Destory(swFilterPipe);
}

/****************************************************************************************************/
/*                                      SwFilterFactory                                             */
/****************************************************************************************************/

SwFilterFactory::SwFilterFactory(VpInterface &vpInterface) :
    m_allocatorCsc(vpInterface),
    m_allocatorRotMir(vpInterface),
    m_allocatorScaling(vpInterface)
{
}

SwFilterFactory::~SwFilterFactory()
{
}

SwFilter *SwFilterFactory::Create(FeatureType type)
{
    SwFilter *swFilter = nullptr;
    switch (type & FEATURE_TYPE_MASK)
    {
    case FeatureTypeCsc:
        swFilter = m_allocatorCsc.Create();
        break;
    case FeatureTypeRotMir:
        swFilter = m_allocatorRotMir.Create();
        break;
    case FeatureTypeScaling:
        swFilter = m_allocatorScaling.Create();
        break;
    default:
        break;
    }

    if (swFilter)
    {
        swFilter->SetFeatureType(type);
    }

    return swFilter;
}

void SwFilterFactory::Destory(SwFilter *&swFilter)
{
    if (nullptr == swFilter)
    {
        return;
    }

    swFilter->Clean();
    swFilter->SetLocation(nullptr);
    FeatureType type = swFilter->GetFeatureType();

    switch (type & FEATURE_TYPE_MASK)
    {
    case FeatureTypeCsc:
    {
        SwFilterCsc *filter = dynamic_cast<SwFilterCsc *>(swFilter);
        m_allocatorCsc.Destory(filter);
        break;
    }
    case FeatureTypeRotMir:
    {
        SwFilterRotMir *filter = dynamic_cast<SwFilterRotMir *>(swFilter);
        m_allocatorRotMir.Destory(filter);
        break;
    }
    case FeatureTypeScaling:
    {
        SwFilterScaling *filter = dynamic_cast<SwFilterScaling *>(swFilter);
        m_allocatorScaling.Destory(filter);
        break;
    }
    default:
        break;
    }

    swFilter = nullptr;
}
