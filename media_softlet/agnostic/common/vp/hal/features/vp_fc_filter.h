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
//! \file     vp_fc_filter.h
//! \brief    Defines the common interface for denoise
//!           this file is for the base interface which is shared by all fc in driver.
//!
#ifndef __VP_FC_FILTER_H__
#define __VP_FC_FILTER_H__
#include "vp_filter.h"
#include "sw_filter.h"

namespace vp {
class VpFcFilter : public VpFilter
{
public:

    VpFcFilter(
        PVP_MHWINTERFACE vpMhwInterface);

    ~VpFcFilter()
    {
        Destroy();
    };

    virtual MOS_STATUS Init() override;

    virtual MOS_STATUS Prepare() override;

    virtual MOS_STATUS Destroy() override;

    virtual MOS_STATUS SetExecuteEngineCaps(
        SwFilterPipe            *executedPipe,
        VP_EXECUTE_CAPS         vpExecuteCaps);

    MOS_STATUS CalculateEngineParams();
    PRENDER_FC_PARAMS GetFcParams()
    {
        return m_renderFcParams;
    }

protected:
    MOS_STATUS GetDefaultScalingMode(VPHAL_SCALING_MODE& defaultScalingMode,SwFilterPipe &executedPipe);
    MOS_STATUS InitLayer(VP_FC_LAYER &layer, bool isInputPipe, int index, SwFilterPipe &executedPipe, VPHAL_SCALING_MODE defaultScalingMode);
    MOS_STATUS InitCompParams(VP_COMPOSITE_PARAMS &compParams, SwFilterPipe &executedPipe);

    MOS_STATUS CalculateCompParams(VP_COMPOSITE_PARAMS &compParams);
    MOS_STATUS CalculateConstantAlpha(uint16_t &alpha, VP_FC_LAYER &layer);
    MOS_STATUS CalculateScalingParams(VP_FC_LAYER *layer, VP_FC_LAYER *target,float &fScaleX, float &fScaleY,
        float &fOffsetX, float &fOffsetY, float &fShiftX , float &fShiftY, RECT &clipedDstRect,
        bool &isChromaUpSamplingNeeded, bool &isChromaDownSamplingNeeded, MHW_SAMPLER_FILTER_MODE &samplerFilterMode,
        float &fStepX, float &fStepY);
    MHW_SAMPLER_FILTER_MODE Get3DSamperFilterMode(VPHAL_SCALING_MODE scalingMode);

    MOS_STATUS AdjustParamsBasedOnFcLimit(VP_COMPOSITE_PARAMS &compParams);

    SwFilterPipe            *m_executedPipe = nullptr;
    PRENDER_FC_PARAMS       m_renderFcParams = nullptr;

MEDIA_CLASS_DEFINE_END(vp__VpFcFilter)
};

struct HW_FILTER_FC_PARAM : public HW_FILTER_PARAM
{
    SwFilterPipe *executedPipe;
};

class HwFilterFcParameter : public HwFilterParameter
{
public:
    static HwFilterParameter *Create(HW_FILTER_FC_PARAM &param, FeatureType featureType);
    HwFilterFcParameter(FeatureType featureType);
    virtual ~HwFilterFcParameter();
    virtual MOS_STATUS ConfigParams(HwFilter &hwFilter);

    MOS_STATUS Initialize(HW_FILTER_FC_PARAM&param);

private:
    HW_FILTER_FC_PARAM m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__HwFilterFcParameter)
};

class VpRenderFcParameter : public VpPacketParameter
{
public:
    static VpPacketParameter *Create(HW_FILTER_FC_PARAM &param);
    VpRenderFcParameter(PVP_MHWINTERFACE pHwInterface, PacketParamFactoryBase *packetParamFactory);
    virtual ~VpRenderFcParameter();

    virtual bool SetPacketParam(VpCmdPacket *pPacket);

private:
    MOS_STATUS Initialize(HW_FILTER_FC_PARAM&params);

    VpFcFilter m_fcFilter;

MEDIA_CLASS_DEFINE_END(vp__VpRenderFcParameter)
};

#define VP_COMP_MAX_LAYERS          8
#define VP_COMP_MAX_PALETTES        2
#define VP_COMP_MAX_PROCAMP         1
#define VP_COMP_MAX_LUMA_KEY        1
#define VP_COMP_MAX_AVS             1
#define VP_COMP_SAMPLER_NEAREST     1
#define VP_COMP_SAMPLER_BILINEAR    2
#define VP_COMP_SAMPLER_LUMAKEY     4
#define VP_COMP_MAX_SAMPLER         (VP_COMP_SAMPLER_NEAREST | VP_COMP_SAMPLER_BILINEAR | VP_COMP_SAMPLER_LUMAKEY)

class PolicyFcFeatureHandler : public PolicyFeatureHandler
{
public:
    PolicyFcFeatureHandler(VP_HW_CAPS &hwCaps) : PolicyFeatureHandler(hwCaps)
    {
        m_Type = FeatureTypeFc;
    }
    virtual ~PolicyFcFeatureHandler()
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
    virtual MOS_STATUS UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        return nullptr;
    }
    virtual MOS_STATUS UpdateUnusedFeature(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);

MEDIA_CLASS_DEFINE_END(vp__PolicyFcFeatureHandler)
};

class PolicyFcHandler : public PolicyFeatureHandler
{
public:
    PolicyFcHandler(VP_HW_CAPS &hwCaps);
    virtual ~PolicyFcHandler();
    virtual bool IsFeatureEnabled(VP_EXECUTE_CAPS vpExecuteCaps);
    virtual HwFilterParameter *CreateHwFilterParam(VP_EXECUTE_CAPS vpExecuteCaps, SwFilterPipe &swFilterPipe, PVP_MHWINTERFACE pHwInterface);
    virtual MOS_STATUS UpdateFeaturePipe(VP_EXECUTE_CAPS caps, SwFilter &feature, SwFilterPipe &featurePipe, SwFilterPipe &executePipe, bool isInputPipe, int index);
    static VpPacketParameter* CreatePacketParam(HW_FILTER_PARAM& param)
    {
        if (param.type != FeatureTypeFcOnRender)
        {
            VP_PUBLIC_ASSERTMESSAGE("Invalid parameter for FC!");
            return nullptr;
        }

        HW_FILTER_FC_PARAM* fcParam = (HW_FILTER_FC_PARAM*)(&param);
        return VpRenderFcParameter::Create(*fcParam);
    }

    virtual MOS_STATUS LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps);

    // true to force all layer to use bilinear if bilinear is needed by any layer.
    // false to use nearest or bilinear based on the requirement of each layer.
    static bool s_forceNearestToBilinearIfBilinearExists;

    static MOS_STATUS IsChromaSamplingNeeded(bool &isChromaUpSamplingNeeded, bool &isChromaDownSamplingNeeded, VPHAL_SURFACE_TYPE surfType, int layerIndex, MOS_FORMAT inputFormat, MOS_FORMAT outputFormat);

protected:
    static bool       IsBobDiEnabled(SwFilterDeinterlace *di, VP_SURFACE &input);
    static bool       IsInterlacedInputSupported(VP_SURFACE &input);
    static MOS_STATUS Get3DSamplerScalingMode(VPHAL_SCALING_MODE &scalingMode, SwFilterSubPipe &pipe, int layerIndex, VP_SURFACE &input, VP_SURFACE &output);
    MOS_STATUS        RemoveTransparentLayers(SwFilterPipe &featurePipe);

private:
    virtual MOS_STATUS AddInputLayerForProcess(bool &bSkip, std::vector<int> &layerIndexes, VPHAL_SCALING_MODE &scalingMode, int index, VP_SURFACE &input, SwFilterSubPipe& pipe, VP_SURFACE &output, VP_EXECUTE_CAPS& caps);

    PacketParamFactory<VpRenderFcParameter> m_PacketParamFactory;

    // Resource counters
    struct
    {
        int32_t                 layers;
        int32_t                 palettes;
        int32_t                 avs;
        int32_t                 procamp;
        int32_t                 lumaKeys;
        int32_t                 sampler;

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

MEDIA_CLASS_DEFINE_END(vp__PolicyFcHandler)
};
}
#endif
