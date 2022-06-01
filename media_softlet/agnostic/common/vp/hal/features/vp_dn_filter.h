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
//! \file     vp_dn_filter.h
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all denoise in driver.
//!
#ifndef __VP_DN_FILTER_H__
#define __VP_DN_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpDnFilter : public VpFilter
{
public:

    VpDnFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpDnFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamDenoise         &dnParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    MOS_STATUS CalculateRenderDnHVSParams();
    PVEBOX_DN_PARAMS GetVeboxParams()
    {
        return m_veboxDnParams;
    }
    PRENDER_DN_HVS_CAL_PARAMS GetRenderDnHVSParams()
    {
        return  &m_renderDnHVSParams;
    }

protected:
    FeatureParamDenoise      m_dnParams = {};
    PVEBOX_DN_PARAMS         m_veboxDnParams = nullptr;
    RENDER_DN_HVS_CAL_PARAMS m_renderDnHVSParams = {};
    VPHAL_HVSDENOISE_PARAMS  m_hvsdenoise        = {};

    SurfaceType m_surfTypeHVSTable = SurfaceTypeHVSTable;
    uint32_t m_HVSQp       = 0;
    uint32_t m_HVSStrength = 0;
    uint32_t m_HVSFormat   = 0;

MEDIA_CLASS_DEFINE_END(vp__VpDnFilter)
};

struct HW_FILTER_DN_PARAM : public HW_FILTER_PARAM
{
    FeatureParamDenoise         dnParams;
};

class HwFilterDnParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_DN_PARAM &param, FeatureType featureType);
    HwFilterDnParameter(FeatureType featureType);
    virtual ~HwFilterDnParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_DN_PARAM&param);

private:
    HW_FILTER_DN_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterDnParameter)
};

class VpVeboxDnParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_DN_PARAM &param);
    VpVeboxDnParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxDnParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_DN_PARAM&params);

    VpDnFilter m_dnFilter;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxDnParameter)
};

class PolicyVeboxDnHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxDnHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxDnHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeDnOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for Vebox De-Noise!");
            return nullptr;
        }

        HW_FILTER_DN_PARAM* dnParam = (HW_FILTER_DN_PARAM*)(&param);
        return VpVeboxDnParameter::Create(*dnParam);
    }

private:
    PacketParamFactory<VpVeboxDnParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyVeboxDnHandler)
};

/*****************DN HVS Calculate Kernel********************/
class VpRenderDnHVSCalParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_DN_PARAM &param);
    VpRenderDnHVSCalParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpRenderDnHVSCalParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_DN_PARAM &params);

    VpDnFilter m_DnFilter;

    MEDIA_CLASS_DEFINE_END(vp__VpRenderDnHVSCalParameter)
};

class PolicyRenderDnHVSCalHandler : public PolicyFeatureHandler
{
public:
    PolicyRenderDnHVSCalHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyRenderDnHVSCalHandler();
    virtual bool               IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual MOS_STATUS         UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter * CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeDnHVSCalOnRender)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid Parameter for Render Dn HVS Calculation!");
            return nullptr;
        }

        HW_FILTER_DN_PARAM * dnParam  = (HW_FILTER_DN_PARAM *)(&param);
        return VpRenderDnHVSCalParameter::Create(*dnParam);
    }

private:
    PacketParamFactory<VpRenderDnHVSCalParameter> m_PacketParamFactory;

    MEDIA_CLASS_DEFINE_END(vp__PolicyRenderDnHVSCalHandler)
};
}
#endif
