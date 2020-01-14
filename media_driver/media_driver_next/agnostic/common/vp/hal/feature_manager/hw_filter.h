/*
* Copyright (c) 2019-2020, Intel Corporation
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
//! \file     hw_filter.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __HW_FILTER_H__
#define __HW_FILTER_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include "vp_vebox_cmd_packet.h"
#include <queue>
#include <vector>
#include "vp_filter.h"
#include "vp_scaling_filter.h"
#include "vp_rot_mir_filter.h"
#include "vp_csc_filter.h"

namespace vp
{

class HwFilter;
class VpInterface;

enum EngineType
{
    EngineTypeInvalid = 0,
    EngineTypeVebox,
    EngineTypeSfc,
    EngineTypeRender,
    // ...
    NumOfEngineType
};

struct HW_FILTER_PARAMS
{
    EngineType Type = EngineTypeInvalid;
    VP_EXECUTE_CAPS vpExecuteCaps = {};
    SwFilterPipe *executedFilters = nullptr;
    std::vector<HwFilterParameter *> Params;
};

struct PACKET_PARAMS
{
    EngineType Type = EngineTypeInvalid;
    std::vector<VpPacketParameter *> Params;
};

class HwFilter
{
public:
    HwFilter(VpInterface &vpInterface, EngineType type);
    virtual ~HwFilter();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Initialize(HW_FILTER_PARAMS &param);
    virtual MOS_STATUS SetPacketParams(VpCmdPacket &package) = 0;

    virtual MOS_STATUS ConfigCscParam(HW_FILTER_CSC_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS ConfigRotMirParam(HW_FILTER_ROT_MIR_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS ConfigScalingParam(HW_FILTER_SCALING_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }

    EngineType GetEngineType()
    {
        return m_Params.Type;
    }

protected:
    VpInterface         &m_vpInterface;
    PACKET_PARAMS       m_Params = {};
    SwFilterPipe        *m_swFilterPipe = nullptr;
    VP_EXECUTE_CAPS     m_vpExecuteCaps = {};
};

class HwFilterVebox: public HwFilter
{
public:
    HwFilterVebox(VpInterface &vpInterface);
    virtual ~HwFilterVebox();
    virtual MOS_STATUS Clean()
    {
        HwFilter::Clean();
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS SetPacketParams(VpCmdPacket &package);
    virtual MOS_STATUS ConfigCscParam(HW_FILTER_CSC_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS ConfigRotMirParam(HW_FILTER_ROT_MIR_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS ConfigScalingParam(HW_FILTER_SCALING_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    HwFilterVebox(VpInterface &vpInterface, EngineType type);
};

class HwFilterSfc: public HwFilterVebox  // VEBOX+SFC
{
public:
    HwFilterSfc(VpInterface &vpInterface);
    virtual ~HwFilterSfc();
    virtual MOS_STATUS Clean()
    {
        HwFilter::Clean();
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS SetPacketParams(VpCmdPacket &package);
    virtual MOS_STATUS ConfigCscParam(HW_FILTER_CSC_PARAM &param)
    {
        if (FeatureTypeCscOnSfc == param.type)
        {
            VpPacketParameter *p = VpSfcCscParameter::Create(param);
            VP_PUBLIC_CHK_NULL_RETURN(p);
            m_Params.Params.push_back(p);
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            return HwFilterVebox::ConfigCscParam(param);
        }
    }
    virtual MOS_STATUS ConfigRotMirParam(HW_FILTER_ROT_MIR_PARAM &param)
    {
        if (FeatureTypeRotMirOnSfc == param.type)
        {
            VpPacketParameter *p = VpSfcRotMirParameter::Create(param);
            VP_PUBLIC_CHK_NULL_RETURN(p);
            m_Params.Params.push_back(p);
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            return HwFilterVebox::ConfigRotMirParam(param);
        }
    }
    virtual MOS_STATUS ConfigScalingParam(HW_FILTER_SCALING_PARAM &param)
    {
        if (FeatureTypeScalingOnSfc == param.type)
        {
            VpPacketParameter *p = VpSfcScalingParameter::Create(param);
            VP_PUBLIC_CHK_NULL_RETURN(p);
            m_Params.Params.push_back(p);
            return MOS_STATUS_SUCCESS;
        }
        else
        {
            return HwFilterVebox::ConfigScalingParam(param);
        }
    }
};

class HwFilterRender: public HwFilter
{
public:
    HwFilterRender(VpInterface &vpInterface);
    virtual ~HwFilterRender();

    virtual MOS_STATUS Clean()
    {
        HwFilter::Clean();
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS SetPacketParams(VpCmdPacket &package);
    virtual MOS_STATUS ConfigCscParam(HW_FILTER_CSC_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS ConfigRotMirParam(HW_FILTER_ROT_MIR_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS ConfigScalingParam(HW_FILTER_SCALING_PARAM &)
    {
        return MOS_STATUS_SUCCESS;
    }
};

}
#endif // !__HW_FILTER_H__
