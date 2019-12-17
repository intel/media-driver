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
//! \file     hw_filter_pipe_factory.cpp
//! \brief    Defines the common interface for hw filter pipe factory
//!
#include "hw_filter_pipe_factory.h"
using namespace vp;

/****************************************************************************************************/
/*                                      HwFilterPipeFactory                                         */
/****************************************************************************************************/

HwFilterPipeFactory::HwFilterPipeFactory()
{
}

HwFilterPipeFactory::~HwFilterPipeFactory()
{
    while (!m_Pool.empty())
    {
        HwFilterPipe *p = m_Pool.back();
        m_Pool.pop_back();
        MOS_Delete(p);
    }
}

MOS_STATUS HwFilterPipeFactory::CreateHwFilterPipe(SwFilterPipe &swfilterPipe,
    Policy &policy, HwFilterPipe *&pHwFilterPipe)
{
    if (m_Pool.empty())
    {
        pHwFilterPipe = MOS_New(HwFilterPipe, m_HwFilterFactroy);
    }
    else
    {
        pHwFilterPipe = m_Pool.back();
        m_Pool.pop_back();
    }

    VP_PUBLIC_CHK_NULL_RETURN(pHwFilterPipe);
    MOS_STATUS status = pHwFilterPipe->Initialize(swfilterPipe, policy);
    if (MOS_FAILED(status))
    {
        ReturnHwFilterPipe(pHwFilterPipe);
    }
    return status;
}

MOS_STATUS HwFilterPipeFactory::CreateHwFilterPipe(VP_PIPELINE_PARAMS &params,
    Policy &policy, HwFilterPipe *&pHwFilterPipe)
{
    if (m_Pool.empty())
    {
        pHwFilterPipe = MOS_New(HwFilterPipe, m_HwFilterFactroy);
    }
    else
    {
        pHwFilterPipe = m_Pool.back();
        m_Pool.pop_back();
    }

    VP_PUBLIC_CHK_NULL_RETURN(pHwFilterPipe);
    MOS_STATUS status = pHwFilterPipe->Initialize(params, policy);
    if (MOS_FAILED(status))
    {
        ReturnHwFilterPipe(pHwFilterPipe);
    }
    return status;
}

void HwFilterPipeFactory::ReturnHwFilterPipe(HwFilterPipe *&pHwfilterPipe)
{
    if (pHwfilterPipe)
    {
        pHwfilterPipe->Clean();
        m_Pool.push_back(pHwfilterPipe);
        pHwfilterPipe = nullptr;
    }
}
