/*
* Copyright (c) 2020-2021, Intel Corporation
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
//! \file     vp_ste_filter.h
//! \brief    Defines the common interface for STE
//!           this file is for the base interface which is shared by all STE in driver.
//!
#ifndef __VP_STE_FILTER_H__
#define __VP_STE_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpSteFilter : public VpFilter
{
public:

    VpSteFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpSteFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamSte         &steParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    PVEBOX_STE_PARAMS GetVeboxParams()
    {
        return m_pVeboxSteParams;
    }

protected:
    FeatureParamSte         m_steParams = {};
    PVEBOX_STE_PARAMS       m_pVeboxSteParams = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpSteFilter)
};

struct HW_FILTER_STE_PARAM : public HW_FILTER_PARAM
{
    FeatureParamSte         steParams;
};

class HwFilterSteParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_STE_PARAM &param, FeatureType featureType);
    HwFilterSteParameter(FeatureType featureType);
    virtual ~HwFilterSteParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_STE_PARAM&param);

private:
    HW_FILTER_STE_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterSteParameter)
};

class VpVeboxSteParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_STE_PARAM &param);
    VpVeboxSteParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxSteParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_STE_PARAM&params);

    VpSteFilter m_steFilter;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxSteParameter)
};

class PolicyVeboxSteHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxSteHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxSteHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeSteOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for Vebox STE!");
            return nullptr;
        }

        HW_FILTER_STE_PARAM* steParam = (HW_FILTER_STE_PARAM*)(&param);
        return VpVeboxSteParameter::Create(*steParam);
    }

private:
    PacketParamFactory<VpVeboxSteParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyVeboxSteHandler)
};
}
#endif
