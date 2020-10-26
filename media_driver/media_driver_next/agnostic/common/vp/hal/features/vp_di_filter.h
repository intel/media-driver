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
//! \file     vp_di_filter.h
//! \brief    Defines the common interface for ace
//!           this file is for the base interface which is shared by all ace in driver.
//!
#ifndef __VP_DI_FILTER_H__
#define __VP_DI_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpDiFilter : public VpFilter
{
public:

    VpDiFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpDiFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        FeatureParamDeinterlace &diParams,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    PVEBOX_DI_PARAMS GetVeboxParams()
    {
        return m_pVeboxDiParams;
    }

protected:
    FeatureParamDeinterlace     m_diParams = {};
    PVEBOX_DI_PARAMS            m_pVeboxDiParams = nullptr;
};

struct HW_FILTER_DI_PARAM : public HW_FILTER_PARAM
{
    FeatureParamDeinterlace     diParams;
};

class HwFilterDiParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_DI_PARAM &param, FeatureType featureType);
    HwFilterDiParameter(FeatureType featureType);
    virtual ~HwFilterDiParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_DI_PARAM&param);

private:
    HW_FILTER_DI_PARAM m_Params = {};
};

class VpVeboxDiParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_DI_PARAM &param);
    VpVeboxDiParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxDiParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_DI_PARAM&params);

    VpDiFilter m_diFilter;
};

class PolicyVeboxDiHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxDiHandler();
    virtual ~PolicyVeboxDiHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeDiOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for Vebox ACE!");
            return nullptr;
        }

        HW_FILTER_DI_PARAM* aceParam = (HW_FILTER_DI_PARAM*)(&param);
        return VpVeboxDiParameter::Create(*aceParam);
    }

private:
    PacketParamFactory<VpVeboxDiParameter> m_PacketParamFactory;
};
}
#endif
