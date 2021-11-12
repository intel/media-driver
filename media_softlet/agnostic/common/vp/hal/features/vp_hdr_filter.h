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
//! \file     vp_hdr_filter.h
//! \brief    Defines the common interface for Hdr
//!           this file is for the base interface which is shared by all Hdr in driver.
//!
#ifndef __VP_hdr_FILTER_H__
#define __VP_hdr_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp
{
class VpHdrFilter : public VpFilter
{
public:
    VpHdrFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    virtual ~VpHdrFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    MOS_STATUS        CalculateEngineParams(
        FeatureParamHdr &HdrParams,
        VP_EXECUTE_CAPS  vpExecuteCaps);
    PVEBOX_HDR_PARAMS GetVeboxParams()
    {
        return m_pVeboxHdrParams;
    }

protected:
    PVEBOX_HDR_PARAMS m_pVeboxHdrParams = nullptr;

MEDIA_CLASS_DEFINE_END(VpHdrFilter)
};

struct HW_FILTER_HDR_PARAM : public HW_FILTER_PARAM
{
    FeatureParamHdr hdrParams;
};

class HwFilterHdrParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_HDR_PARAM &param, FeatureType featureType);
    HwFilterHdrParameter(FeatureType featureType);
    virtual ~HwFilterHdrParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_HDR_PARAM &param);

private:
    HW_FILTER_HDR_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(HwFilterHdrParameter)
};

class VpVeboxHdrParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_HDR_PARAM &param);
    VpVeboxHdrParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpVeboxHdrParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_HDR_PARAM &params);

    VpHdrFilter m_HdrFilter;

MEDIA_CLASS_DEFINE_END(VpVeboxHdrParameter)
};

class PolicyVeboxHdrHandler : public PolicyFeatureHandler
{
public:
    PolicyVeboxHdrHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyVeboxHdrHandler();
    virtual bool               IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);

    static VpPacketParameter * CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeHdrOnVebox)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid Parameter for VEBOX Hdr!");
            return nullptr;
        }

        HW_FILTER_HDR_PARAM *HdrParam = (HW_FILTER_HDR_PARAM *)(&param);
        return VpVeboxHdrParameter::Create(*HdrParam);
    }

private:
    PacketParamFactory<VpVeboxHdrParameter> m_PacketParamFactory;

MEDIA_CLASS_DEFINE_END(PolicyVeboxHdrHandler)
};
}  // namespace vp
#endif
