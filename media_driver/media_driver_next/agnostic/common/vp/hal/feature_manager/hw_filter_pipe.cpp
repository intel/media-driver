/*
* Copyright (c) 2019 - 2020, Intel Corporation
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
//! \file     hw_filter_pipe.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "hw_filter_pipe.h"
#include "vp_resource_manager.h"
#include "vp_obj_factories.h"
using namespace vp;

/****************************************************************************************************/
/*                                      HwFilterPipe                                                */
/****************************************************************************************************/
HwFilterPipe::HwFilterPipe(VpInterface &vpInterface) : m_vpInterface(vpInterface)
{
}

HwFilterPipe::~HwFilterPipe()
{
    Clean();
}

MOS_STATUS HwFilterPipe::Initialize(SwFilterPipe &swFilterPipe, Policy &policy)
{
    MOS_STATUS status = MOS_STATUS_SUCCESS;
    SwFilterPipe &subSwFilterPipe = swFilterPipe;
    HwFilter *pHwFilter = nullptr;

    Clean();

    status = policy.CreateHwFilter(subSwFilterPipe, pHwFilter);

    if (MOS_FAILED(status))
    {
        return status;
    }

    while (pHwFilter)
    {
        status = AddHwFilter(*pHwFilter);

        if (MOS_FAILED(status))
        {
            return status;
        }

        status = policy.CreateHwFilter(subSwFilterPipe, pHwFilter);

        if (MOS_FAILED(status))
        {
            return status;
        }
    }
    return status;
}

void HwFilterPipe::Clean()
{
    while (!m_Pipe.empty())
    {
        HwFilter *p = m_Pipe.back();
        m_Pipe.pop_back();
        m_vpInterface.GetHwFilterFactory().Destory(p);
    }
}

MOS_STATUS HwFilterPipe::AddHwFilter(HwFilter &hwFilter)
{
    m_Pipe.push_back(&hwFilter);
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HwFilterPipe::InitPacketPipe(PacketPipe &packetPipe)
{
    for (std::vector<HwFilter*>::iterator it = m_Pipe.begin(); it!= m_Pipe.end(); ++it)
    {
        if (nullptr == *it)
        {
            return MOS_STATUS_NULL_POINTER;
        }
        packetPipe.AddPacket(**it);
    }
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS HwFilterPipe::UpdateResources()
{
    return MOS_STATUS_SUCCESS;
}

uint32_t HwFilterPipe::HwFilterCount()
{
    return m_Pipe.size();
}

EngineType HwFilterPipe::GetEngineType(uint32_t index)
{
    if (index >= m_Pipe.size() || nullptr == m_Pipe[index])
    {
        return EngineTypeInvalid;
    }

    return m_Pipe[index]->GetEngineType();
}