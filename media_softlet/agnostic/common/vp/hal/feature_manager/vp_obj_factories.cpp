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
#include "sw_filter_handle.h"
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
    VP_FUNC_CALL();

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
    VP_FUNC_CALL();

    return m_allocator.Destory(pHwfilterPipe);
}

/****************************************************************************************************/
/*                                      HwFilterFactory                                             */
/****************************************************************************************************/

HwFilterFactory::HwFilterFactory(VpInterface &vpInterface) : m_allocatorVebox(vpInterface), m_allocatorVeboxSfc(vpInterface), m_allocatorRender(vpInterface)
{
}

HwFilterFactory::~HwFilterFactory()
{
}

HwFilter *HwFilterFactory::Create(HW_FILTER_PARAMS &param)
{
    VP_FUNC_CALL();

    HwFilter *p = nullptr;
    switch (param.Type)
    {
    case EngineTypeVebox:
        p = m_allocatorVebox.Create();
        break;
    case EngineTypeVeboxSfc:
        p = m_allocatorVeboxSfc.Create();
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
    VP_FUNC_CALL();

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
        case EngineTypeVeboxSfc:
        {
            HwFilterVeboxSfc *p = dynamic_cast<HwFilterVeboxSfc*>(pHwFilter);
            if (p)
            {
                m_allocatorVeboxSfc.Destory(p);
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

SwFilterPipeFactory::SwFilterPipeFactory(VpInterface &vpInterface) :
    m_allocator(vpInterface),
    m_vpInterface(vpInterface)
{
}

SwFilterPipeFactory::~SwFilterPipeFactory()
{
}

int SwFilterPipeFactory::GetPipeCountForProcessing(VP_PIPELINE_PARAMS &params)
{
    VP_FUNC_CALL();

    int pipeCnt = 1;
    int featureCnt = 0;
    auto featureHander = *m_vpInterface.GetSwFilterHandlerMap();
    for (auto &handler : featureHander)
    {
        int cnt = handler.second->GetPipeCountForProcessing(params);
        if (cnt > 1)
        {
            pipeCnt = cnt;
            featureCnt++;
        }
    }
    if (featureCnt > 1)
    {
        VP_PUBLIC_ASSERTMESSAGE("Invalid usage, only support 1 feature for multi-pipe.");
        return 0;
    }
    return pipeCnt;
}

MOS_STATUS SwFilterPipeFactory::Update(VP_PIPELINE_PARAMS &params, int index)
{
    VP_FUNC_CALL();

    auto featureHander = *m_vpInterface.GetSwFilterHandlerMap();
    for (auto &handler : featureHander)
    {
        VP_PUBLIC_CHK_STATUS_RETURN(handler.second->UpdateParamsForProcessing(params, index));
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipeFactory::Create(PVP_PIPELINE_PARAMS params, std::vector<SwFilterPipe *> &swFilterPipe, VpPipelineParamFactory *pipelineParamFactory)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);
    VP_PUBLIC_CHK_NULL_RETURN(pipelineParamFactory);
    int pipeCnt = GetPipeCountForProcessing(*params);
    if (pipeCnt == 0)
    {
        return MOS_STATUS_INVALID_PARAMETER;
    }

    MT_LOG1(MT_VP_HAL_SWWFILTER, MT_NORMAL, MT_VP_HAL_PIPE_CNT, pipeCnt);

    for (int index = 0; index < pipeCnt; index++)
    {
        VP_PIPELINE_PARAMS *tempParams = pipelineParamFactory->Clone(params);
        VP_PUBLIC_CHK_NULL_RETURN(tempParams);
        VP_PUBLIC_CHK_STATUS_RETURN(Update(*tempParams, index));

        SwFilterPipe *pipe = m_allocator.Create();
        if (!pipe)
        {
            pipelineParamFactory->Destroy(tempParams);
            VP_PUBLIC_CHK_STATUS_RETURN(MOS_STATUS_NULL_POINTER);
        }

        FeatureRule featureRule;
        MOS_STATUS status = pipe->Initialize(*tempParams, featureRule);

        VP_PUBLIC_CHK_STATUS_RETURN(pipelineParamFactory->Destroy(tempParams));

        if (MOS_FAILED(status))
        {
            m_allocator.Destory(pipe);
            return status;
        }

        swFilterPipe.push_back(pipe);
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS SwFilterPipeFactory::Create(VEBOX_SFC_PARAMS *params, std::vector<SwFilterPipe*> &swFilterPipe)
{
    VP_FUNC_CALL();

    VP_PUBLIC_CHK_NULL_RETURN(params);
    SwFilterPipe *pipe = m_allocator.Create();
    VP_PUBLIC_CHK_NULL_RETURN(pipe);

    MOS_STATUS status = pipe->Initialize(*params);

    if (MOS_FAILED(status))
    {
        m_allocator.Destory(pipe);
        return status;
    }

    swFilterPipe.push_back(pipe);
    return status;
}

MOS_STATUS SwFilterPipeFactory::Create(SwFilterPipe *&swFilterPipe)
{
    VP_FUNC_CALL();

    swFilterPipe = m_allocator.Create();
    VP_PUBLIC_CHK_NULL_RETURN(swFilterPipe);

    return MOS_STATUS_SUCCESS;
}

void SwFilterPipeFactory::Destory(SwFilterPipe *&swFilterPipe)
{
    VP_FUNC_CALL();

    m_allocator.Destory(swFilterPipe);
}
