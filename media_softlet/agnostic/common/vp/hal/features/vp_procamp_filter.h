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
//! \file     vp_procamp_filter.h
//! \brief    Defines the common interface for Procamp
//!           this file is for the base interface which is shared by all Procamp in driver.
//!
#ifndef __VP_PROCAMP_FILTER_H__
#define __VP_PROCAMP_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpProcampFilter : public VpFilter
{
public:

    VpProcampFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpProcampFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamProcamp     &procampParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    PVEBOX_PROCAMP_PARAMS GetVeboxParams()
    {
        return m_pVeboxProcampParams;
    }

protected:
    FeatureParamProcamp     m_procampParams = {};
    PVEBOX_PROCAMP_PARAMS   m_pVeboxProcampParams = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpProcampFilter)
};

struct HW_FILTER_PROCAMP_PARAM : public HW_FILTER_PARAM
{
    FeatureParamProcamp         procampParams;
};

class HwFilterProcampParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_PROCAMP_PARAM &param, FeatureType featureType);
    HwFilterProcampParameter(FeatureType featureType);
    virtual ~HwFilterProcampParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_PROCAMP_PARAM&param);

private:
    HW_FILTER_PROCAMP_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterProcampParameter)
};

class VpVeboxProcampParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_PROCAMP_PARAM &param);
    VpVeboxProcampParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxProcampParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_PROCAMP_PARAM&params);

    VpProcampFilter m_procampFilter;

MEDIA_CLASS_DEFINE_END(vp__VpVeboxProcampParameter)
};

class PolicyVeboxProcampHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxProcampHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxProcampHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeProcampOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for Vebox Procamp!");
            return nullptr;
        }

        HW_FILTER_PROCAMP_PARAM* procampParam = (HW_FILTER_PROCAMP_PARAM*)(&param);
        return VpVeboxProcampParameter::Create(*procampParam);
    }

private:
    PacketParamFactory<VpVeboxProcampParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyVeboxProcampHandler)
};
}
#endif
