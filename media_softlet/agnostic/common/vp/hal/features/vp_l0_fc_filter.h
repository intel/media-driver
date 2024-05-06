/*
* Copyright (c) 2024, Intel Corporation
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
//! \file     vp_l0_fc_filter.h
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all l0 fc in driver.


#ifndef __VP_L0_FC_FILTER_H__
#define __VP_L0_FC_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp{

struct L0_FC_KERNEL_PARAM
{
    KERNEL_ARGS kernelArgs;
    std::string kernelName;
    VpKernelID  kernelId;
    uint32_t    threadWidth;
    uint32_t    threadHeight;
    uint32_t    localWidth;
    uint32_t    localHeight;
    void        Init();
};

using L0_FC_KERNEL_PARAMS = std::vector<L0_FC_KERNEL_PARAM>;
struct _RENDER_L0_FC_PARAMS
{
    L0_FC_KERNEL_PARAMS fc_kernelParams = {};
    void                            Init();
};
using RENDER_L0_FC_PARAMS  = _RENDER_L0_FC_PARAMS;
using PRENDER_L0_FC_PARAMS = RENDER_L0_FC_PARAMS *;

class VpL0FcFilter : public VpFilter
{
public:
    VpL0FcFilter(PVP_MHWINTERFACE vpMhwInterface);

    ~VpL0FcFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        SwFilterPipe   *executingPipe,
        VP_EXECUTE_CAPS vpExecuteCaps);

    MOS_STATUS        CalculateEngineParams();
    PRENDER_L0_FC_PARAMS GetFcParams()
    {
        return m_renderL0FcParams;
    }

protected:
    MOS_STATUS InitCompParams(L0_FC_KERNEL_PARAMS &compParams, SwFilterPipe &executingPipe);
    MOS_STATUS AddScalingKrn(SwFilterScaling *scaling, SurfaceType inputSurf, SurfaceType outputSurf, L0_FC_KERNEL_PARAM &param);

    SwFilterPipe        *m_executingPipe    = nullptr;
    PRENDER_L0_FC_PARAMS m_renderL0FcParams = nullptr;

    KERNEL_INDEX_ARG_MAP m_scalingKrnArgs;

MEDIA_CLASS_DEFINE_END(vp__VpL0FcFilter)
};

struct HW_FILTER_L0_FC_PARAM : public HW_FILTER_PARAM
{
    SwFilterPipe *executingPipe;
};

class HwFilterL0FcParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_L0_FC_PARAM &param, FeatureType featureType);
    HwFilterL0FcParameter(FeatureType featureType);
    virtual ~HwFilterL0FcParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_L0_FC_PARAM &param);

private:
    HW_FILTER_L0_FC_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterL0FcParameter)
};

class VpRenderL0FcParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_L0_FC_PARAM &param);
    VpRenderL0FcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpRenderL0FcParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_L0_FC_PARAM &params);

    VpL0FcFilter m_fcFilter;

MEDIA_CLASS_DEFINE_END(vp__VpRenderL0FcParameter)
};

class PolicyL0FcFeatureHandler : public PolicyFeatureHandler
{
public:
    PolicyL0FcFeatureHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
    {
        m_Type = FeatureTypeFc;
    }
    virtual ~PolicyL0FcFeatureHandler()
    {
    }
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps)
    {
        // Not create hwFilters for single FC features.
        return false;
    }
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface)
    {
        return nullptr;
    }
    virtual MOS_STATUS        UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter *CreatePacketParam(HW_FILTER_PARAM &param)
    {
        return nullptr;
    }
    virtual MOS_STATUS UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);

MEDIA_CLASS_DEFINE_END(vp__PolicyL0FcFeatureHandler)
};

class PolicyL0FcHandler : public PolicyFeatureHandler
{
public:
    PolicyL0FcHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyL0FcHandler();
    virtual bool               IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual MOS_STATUS         UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter  *CreatePacketParam(HW_FILTER_PARAM &param)
    {
        if (param.type != FeatureTypeFcOnRender)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for FC!");
            return nullptr;
        }

        HW_FILTER_L0_FC_PARAM *fcParam = (HW_FILTER_L0_FC_PARAM *)(&param);
        return VpRenderL0FcParameter::Create(*fcParam);
    }

    virtual MOS_STATUS LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, VP_EXECUTE_CAPS &caps);

private:
    MOS_STATUS         RemoveTransparentLayers(SwFilterPipe &featurePipe);
    virtual MOS_STATUS AddInputLayerForProcess(bool &bSkip, std::vector<int> &layerIndexes, VPHAL_SCALING_MODE &scalingMode, int index, VP_SURFACE &input, SwFilterSubPipe &pipe, VP_SURFACE &output, VP_EXECUTE_CAPS &caps);

    PacketParamFactory<VpRenderL0FcParameter> m_PacketParamFactory;

    // Resource counters
    struct
    {
        int32_t layers;
        int32_t palettes;
        int32_t avs;
        int32_t procamp;
        int32_t lumaKeys;
        int32_t sampler;

        void Reset(bool isAvsSamplerSupported)
        {
            // Next step to init it from hw caps object.
            layers   = VP_COMP_MAX_LAYERS;
            palettes = VP_COMP_MAX_PALETTES;
            procamp  = VP_COMP_MAX_PROCAMP;
            lumaKeys = VP_COMP_MAX_LUMA_KEY;
            avs      = isAvsSamplerSupported ? VP_COMP_MAX_AVS : 0;
            sampler  = VP_COMP_MAX_SAMPLER;
        }
    } m_resCounter = {};

MEDIA_CLASS_DEFINE_END(vp__PolicyL0FcHandler)
};

}

#endif