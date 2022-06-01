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
//! \file     vp_tcc_filter.h
//! \brief    Defines the common interface for tcc
//!           this file is for the base interface which is shared by all tcc in driver.
//!
#ifndef __VP_TCC_FILTER_H__
#define __VP_TCC_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpTccFilter : public VpFilter
{
public:

    VpTccFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpTccFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamTcc         &tccParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    PVEBOX_TCC_PARAMS GetVeboxParams()
    {
        return m_pVeboxTccParams;
    }

protected:
    FeatureParamTcc         m_tccParams = {};
    PVEBOX_TCC_PARAMS       m_pVeboxTccParams = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpTccFilter)
};

struct HW_FILTER_TCC_PARAM : public HW_FILTER_PARAM
{
    FeatureParamTcc         tccParams;
};

class HwFilterTccParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_TCC_PARAM &param, FeatureType featureType);
    HwFilterTccParameter(FeatureType featureType);
    virtual ~HwFilterTccParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_TCC_PARAM&param);

private:
    HW_FILTER_TCC_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterTccParameter)
};

class VpVeboxTccParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_TCC_PARAM &param);
    VpVeboxTccParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxTccParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_TCC_PARAM&params);

    VpTccFilter m_tccFilter;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxTccParameter)
};

class PolicyVeboxTccHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxTccHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxTccHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeTccOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for Vebox TCC!");
            return nullptr;
        }

        HW_FILTER_TCC_PARAM* tccParam = (HW_FILTER_TCC_PARAM*)(&param);
        return VpVeboxTccParameter::Create(*tccParam);
    }

private:
    PacketParamFactory<VpVeboxTccParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyVeboxTccHandler)
};
}
#endif
