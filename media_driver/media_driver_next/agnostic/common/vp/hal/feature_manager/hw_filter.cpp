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
//! \file     hw_filter.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "hw_filter.h"

using namespace vp;

/****************************************************************************************************/
/*                                      HwFilter                                                    */
/****************************************************************************************************/

HwFilter::HwFilter(EngineType type)
{
    m_Params.Type = type;
}

HwFilter::~HwFilter()
{
    Clean();
}

MOS_STATUS HwFilter::Initialize(HW_FILTER_PARAMS &param)
{
    bool bRet = true;

    Clean();

    m_pVpParams = param.pVpParams;
    m_vpExecuteCaps = param.vpExecuteCaps;
    m_Params.Type = param.Type;

    std::vector<HwFilterParameter *>::iterator it = param.Params.begin();
    for (; it != param.Params.end(); ++it)
    {
        VP_PUBLIC_CHK_STATUS_RETURN((*it)->ConfigParams(*this));
    }
    return MOS_STATUS_SUCCESS;
}

void HwFilter::Clean()
{
    std::vector<VpPacketParameter *>::iterator it = m_Params.Params.begin();
    for (; it != m_Params.Params.end(); ++it)
    {
        VpPacketParameter *p = *it;
        VpPacketParameter::Destory(p);
    }
    m_Params.Params.clear();
}

/****************************************************************************************************/
/*                                      HwFilterVebox                                               */
/****************************************************************************************************/

HwFilterVebox::HwFilterVebox() : HwFilter(EngineTypeVebox)
{
}

HwFilterVebox::HwFilterVebox(EngineType type) : HwFilter(type)
{
}

HwFilterVebox::~HwFilterVebox()
{
}

MOS_STATUS HwFilterVebox::SetPacketParams(VpCmdPacket &packet)
{
    bool bRet = true;

    VP_PUBLIC_CHK_STATUS_RETURN(packet.PacketInit(m_pVpParams, m_vpExecuteCaps));

    std::vector<VpPacketParameter *>::iterator it = m_Params.Params.begin();
    for (; it != m_Params.Params.end(); ++it)
    {
        bRet = bRet && (*it)->SetPacketParam(&packet);
    }
    return bRet ? MOS_STATUS_SUCCESS : MOS_STATUS_UNKNOWN;
}

/****************************************************************************************************/
/*                                      HwFilterSfc                                                 */
/****************************************************************************************************/

HwFilterSfc::HwFilterSfc() : HwFilterVebox(EngineTypeSfc)
{}

HwFilterSfc::~HwFilterSfc()
{}

MOS_STATUS HwFilterSfc::SetPacketParams(VpCmdPacket &packet)
{
    return HwFilterVebox::SetPacketParams(packet);
}

/****************************************************************************************************/
/*                                      HwFilterRender                                              */
/****************************************************************************************************/

HwFilterRender::HwFilterRender() : HwFilter(EngineTypeRender)
{}

HwFilterRender::~HwFilterRender()
{}

MOS_STATUS HwFilterRender::SetPacketParams(VpCmdPacket &packet)
{
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                      HwFilterFactory                                             */
/****************************************************************************************************/

HwFilterFactory::HwFilterFactory()
{
}

HwFilterFactory::~HwFilterFactory()
{
    HwFilter *p = nullptr;
    while (p = GetIdleHwFilter(m_PoolVebox))
    {
        MOS_Delete(p);
    }
    while (p = GetIdleHwFilter(m_PoolSfc))
    {
        MOS_Delete(p);
    }
    while (p = GetIdleHwFilter(m_PoolRender))
    {
        MOS_Delete(p);
    }
}

HwFilter *HwFilterFactory::GetHwFilter(HW_FILTER_PARAMS &param)
{
    HwFilter *p = nullptr;
    switch (param.Type)
    {
    case EngineTypeVebox:
        p = GetIdleHwFilter(m_PoolVebox);
        if (nullptr == p)
        {
            p = MOS_New(HwFilterVebox);
        }
        break;
    case EngineTypeSfc:
        p = GetIdleHwFilter(m_PoolSfc);
        if (nullptr == p)
        {
            p = MOS_New(HwFilterSfc);
        }
        break;
    case EngineTypeRender:
        p = GetIdleHwFilter(m_PoolRender);
        if (nullptr == p)
        {
            p = MOS_New(HwFilterRender);
        }
        break;
    default:
        return nullptr;
        break;
    }
    if (p)
    {
        if (MOS_FAILED(p->Initialize(param)))
        {
            ReturnHwFilter(p);
            return nullptr;
        }
    }
    return p;
}

void HwFilterFactory::ReturnHwFilter(HwFilter *&pHwFilter)
{
    if (pHwFilter)
    {
        pHwFilter->Clean();

        switch (pHwFilter->GetEngineType())
        {
        case EngineTypeVebox:
            m_PoolVebox.push(pHwFilter);
            break;
        case EngineTypeSfc:
            m_PoolSfc.push(pHwFilter);
            break;
        case EngineTypeRender:
            m_PoolRender.push(pHwFilter);
            break;
        default:
            MOS_Delete(pHwFilter);
            return;
        }
        pHwFilter = nullptr;
    }
}

HwFilter *HwFilterFactory::GetIdleHwFilter(std::queue<HwFilter *> &pool)
{
    if (pool.empty())
    {
        return nullptr;
    }
    HwFilter *pHwFilter = pool.front();
    pool.pop();
    return pHwFilter;
}
