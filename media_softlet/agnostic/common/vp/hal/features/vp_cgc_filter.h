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
//! \file     vp_cgc_filter.h
//! \brief    Defines the common interface for Color Gamut Compression and Expansion
//!           this file is for the base interface which is shared by all CGC in driver.
//!
#ifndef __VP_CGC_FILTER_H__
#define __VP_CGC_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpCgcFilter : public VpFilter
{
public:

    VpCgcFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpCgcFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamCgc         &cgcParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    PVEBOX_CGC_PARAMS GetVeboxParams()
    {
        return m_pVeboxCgcParams;
    }

protected:
    FeatureParamCgc         m_cgcParams = {};
    PVEBOX_CGC_PARAMS       m_pVeboxCgcParams = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpCgcFilter)
};

struct HW_FILTER_CGC_PARAM : public HW_FILTER_PARAM
{
    FeatureParamCgc         cgcParams;
};

class HwFilterCgcParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_CGC_PARAM &param, FeatureType featureType);
    HwFilterCgcParameter(FeatureType featureType);
    virtual ~HwFilterCgcParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_CGC_PARAM&param);

private:
    HW_FILTER_CGC_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterCgcParameter)
};

class VpVeboxCgcParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_CGC_PARAM &param);
    VpVeboxCgcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxCgcParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_CGC_PARAM&params);

    VpCgcFilter m_cgcFilter;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxCgcParameter)
};

class PolicyVeboxCgcHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxCgcHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxCgcHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeCgcOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for Vebox CGC!");
            return nullptr;
        }

        HW_FILTER_CGC_PARAM* cgcParam = (HW_FILTER_CGC_PARAM*)(&param);
        return VpVeboxCgcParameter::Create(*cgcParam);
    }

private:
    PacketParamFactory<VpVeboxCgcParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyVeboxCgcHandler)
};
}
#endif
