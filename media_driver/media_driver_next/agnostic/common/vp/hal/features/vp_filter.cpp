/*
* Copyright (c) 2018-2020, Intel Corporation
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
//! \file     vp_fliter.cpp
//! \brief    Defines the common interface for vp filters
//!           this file is for the base interface which is shared by all features.
//!

#include "vp_filter.h"

using namespace vp;
/****************************************************************************************************/
/*                                      VpFilter                                                      */
/****************************************************************************************************/
VpFilter::VpFilter(PVP_MHWINTERFACE vpMhwInterface) :
    m_pvpMhwInterface(vpMhwInterface)
{

}

/****************************************************************************************************/
/*                                     HwFilter Parameters                                          */
/****************************************************************************************************/
HwFilterParameter::HwFilterParameter(FeatureType featureType) : m_FeatureType(featureType)
{
}

HwFilterParameter::~HwFilterParameter()
{
}

/****************************************************************************************************/
/*                                      Packet Parameters                                           */
/****************************************************************************************************/
VpPacketParameter::VpPacketParameter(PacketParamFactoryBase *packetParamFactory) : m_packetParamFactory(packetParamFactory)
{
}

VpPacketParameter::~VpPacketParameter()
{
}

void VpPacketParameter::Destory(VpPacketParameter *&p)
{
    if (nullptr == p)
    {
        return;
    }

    PacketParamFactoryBase *packetParamFactory = p->m_packetParamFactory;
    if (nullptr == packetParamFactory)
    {
        MOS_Delete(p);
        return;
    }

    packetParamFactory->ReturnPacketParameter(p);
}

/****************************************************************************************************/
/*                                  Policy Feature Handler                                          */
/****************************************************************************************************/
PolicyFeatureHandler::PolicyFeatureHandler()
{
}

PolicyFeatureHandler::~PolicyFeatureHandler()
{
    while(!m_Pool.empty())
    {
        HwFilterParameter *p = m_Pool.back();
        m_Pool.pop_back();
        MOS_Delete(p);
    }
}

bool PolicyFeatureHandler::IsFeatureEnabled(SwFilterPipe &swFilterPipe)
{
    return false;
}

HwFilterParameter *PolicyFeatureHandler::CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
{
    return nullptr;
}

bool PolicyFeatureHandler::IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
{
    return false;
}

FeatureType PolicyFeatureHandler::GetType()
{
    return m_Type;
}

HwFilterParameter *PolicyFeatureHandler::GetHwFeatureParameterFromPool()
{
    if (m_Pool.empty())
    {
        return nullptr;
    }
    HwFilterParameter *p = m_Pool.back();
    m_Pool.pop_back();
    return p;
}

MOS_STATUS PolicyFeatureHandler::ReleaseHwFeatureParameter(HwFilterParameter *&pParam)
{
    VP_PUBLIC_CHK_NULL_RETURN(pParam);
    m_Pool.push_back(pParam);
    pParam = nullptr;
    return MOS_STATUS_SUCCESS;
}

/****************************************************************************************************/
/*                                  Packet Param Factory Base                                       */
/****************************************************************************************************/
PacketParamFactoryBase::PacketParamFactoryBase()
{
}

PacketParamFactoryBase::~PacketParamFactoryBase()
{
    while (!m_Pool.empty())
    {
        VpPacketParameter *p = m_Pool.back();
        m_Pool.pop_back();
        MOS_Delete(p);
    }
}

VpPacketParameter *PacketParamFactoryBase::GetPacketParameter(PVP_MHWINTERFACE pHwInterface)
{
    return nullptr;
}

void PacketParamFactoryBase::ReturnPacketParameter(VpPacketParameter *&p)
{
    if (p)
    {
        m_Pool.push_back(p);
        p = nullptr;
    }
}
