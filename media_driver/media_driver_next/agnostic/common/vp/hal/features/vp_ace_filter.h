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
//! \file     vp_ace_filter.h
//! \brief    Defines the common interface for ace
//!           this file is for the base interface which is shared by all ace in driver.
//!
#ifndef __VP_ACE_FILTER_H__
#define __VP_ACE_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpAceFilter : public VpFilter
{
public:

    VpAceFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpAceFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamAce         &aceParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    PVEBOX_ACE_PARAMS GetVeboxParams()
    {
        return m_pVeboxAceParams;
    }

protected:
    FeatureParamAce         m_aceParams = {};
    PVEBOX_ACE_PARAMS       m_pVeboxAceParams = nullptr;
};


struct HW_FILTER_ACE_PARAM
{
    FeatureType             type;
    PVP_MHWINTERFACE        pHwInterface;
    VP_EXECUTE_CAPS         vpExecuteCaps;
    PacketParamFactoryBase *pPacketParamFactory;
    FeatureParamAce         aceParams;
};

class HwFilterAceParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_ACE_PARAM &param, FeatureType featureType);
    HwFilterAceParameter(FeatureType featureType);
    virtual ~HwFilterAceParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_ACE_PARAM&param);

private:
    HW_FILTER_ACE_PARAM m_Params = {};
};

class VpVeboxAceParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_ACE_PARAM &param);
    VpVeboxAceParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxAceParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_ACE_PARAM&params);

    VpAceFilter m_aceFilter;
};

class PolicyVeboxAceHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxAceHandler();
    virtual ~PolicyVeboxAceHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

private:
    PacketParamFactory<VpVeboxAceParameter> m_PacketParamFactory;
};
}
#endif
