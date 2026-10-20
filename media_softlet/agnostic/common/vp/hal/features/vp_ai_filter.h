/*
* Copyright (c) 2025, Intel Corporation
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
//! \file     vp_ai_filter.h
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all ai filter in driver.

#ifndef __VP_AI_FILTER_H__
#define __VP_AI_FILTER_H__
#include "vp_filter.h"
#include "vp_graph_manager.h"
#include "vp_npu_cmd_packet.h"

namespace vp
{

class VpAiFilter : public VpFilter
{
public:
    VpAiFilter(PVP_MHWINTERFACE vpMhwInterface);

    ~VpAiFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        SwFilterPipe   *executingPipe,
        VP_EXECUTE_CAPS vpExecuteCaps);

    MOS_STATUS        CalculateEngineParams(VpGraphManager *graphManager = nullptr);
    PRENDER_AI_PARAMS GetAiParams()
    {
        return m_renderAiParams;
    };
    NPU_PACKET_PARAM &GetNpuParams()
    {
        return m_npuAiParams;
    }

private:
    struct AI_KRN_ARG
    {
        KRN_ARG  krnArg       = {};
        uint32_t reservedSize = 0;
    };
    using AI_KERNEL_INDEX_ARG_MAP              = std::map<uint32_t,AI_KRN_ARG>;
    using AI_MULTI_LAYERS_KERNEL_INDEX_ARG_MAP = std::map<uint32_t,AI_KERNEL_INDEX_ARG_MAP>;
    using FEATURE_AI_KERNEL_ARG_MAP = std::map<FeatureType, AI_MULTI_LAYERS_KERNEL_INDEX_ARG_MAP>;

    MOS_STATUS SetConfig(SwFilterPipe &executingPipe, AI_KERNEL_CONFIG &config);
    MOS_STATUS InitKrnParams(AI_KERNEL_PARAMS &krnParams, SwFilterPipe &executingPipe);
    MOS_STATUS InitNpuParams(NPU_PACKET_PARAM &npuParams, SwFilterPipe &executingPipe, VpGraphManager *graphManager);

protected:
    SwFilterPipe             *m_executingPipe      = nullptr;
    PRENDER_AI_PARAMS         m_renderAiParams     = nullptr;
    NPU_PACKET_PARAM          m_npuAiParams        = {};
    FEATURE_AI_KERNEL_ARG_MAP m_featureAiKrnArgMap = {};

MEDIA_CLASS_DEFINE_END(vp__VpAiFilter)
};

struct HW_FILTER_AI_PARAM : public HW_FILTER_PARAM
{
    SwFilterPipe   *executingPipe = nullptr;
    VpGraphManager *graphManager  = nullptr;
};

class HwFilterAiParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_AI_PARAM &param, FeatureType featureType);
    HwFilterAiParameter(FeatureType featureType);
    virtual ~HwFilterAiParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_AI_PARAM &param);

private:
    HW_FILTER_AI_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterAiParameter)
};

class VpRenderAiParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_AI_PARAM &param);
    VpRenderAiParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpRenderAiParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_AI_PARAM &params);

    VpAiFilter m_filter;

MEDIA_CLASS_DEFINE_END(vp__VpRenderAiParameter)
};

class VpNpuAiParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_AI_PARAM &param);
    VpNpuAiParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpNpuAiParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_AI_PARAM &params);

    VpAiFilter m_filter;

MEDIA_CLASS_DEFINE_END(vp__VpNpuAiParameter)
};

class PolicyAiHandler : public PolicyFeatureHandler
{
public:
    PolicyAiHandler(VP_HW_CAPS &hwCaps, VpGraphManager *graphManager);
    virtual ~PolicyAiHandler();
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface) override;
    virtual bool               IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps) override;
    virtual MOS_STATUS         UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index) override;
    static VpPacketParameter  *CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeAi)
        {
            return nullptr;
        }

        HW_FILTER_AI_PARAM *aiParam = (HW_FILTER_AI_PARAM *)(&param);
        if (param.vpExecuteCaps.bNpu)
        {
            return VpNpuAiParameter::Create(*aiParam);
        }
        else
        {
            return VpRenderAiParameter::Create(*aiParam);
        }
    }

private:
    VpGraphManager                         *m_graphManager = nullptr;
    PacketParamFactory<VpRenderAiParameter> m_renderPacketParamFactory;
    PacketParamFactory<VpNpuAiParameter>    m_npuPacketParamFactory;

MEDIA_CLASS_DEFINE_END(vp__PolicyAiHandler)
};

}  // namespace vp

#endif