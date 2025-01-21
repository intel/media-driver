/*
* Copyright (c) 2019-2023, Intel Corporation
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
//! \file     policy.h
//! \brief    Defines the common interface for vp features manager
//! \details  The policy is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __POLICY_H__
#define __POLICY_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"
#include "vp_feature_caps.h"

#include "hw_filter.h"
#include "sw_filter_pipe.h"
#include "vp_resource_manager.h"
#include <map>

namespace vp
{
#define ENGINE_MUST_MASK(supported)    (supported & 0x02)
#define ENGINE_SUPPORT_MASK(supported) (supported & 0x3)
#define ENGINE_MUST(supported)         (supported << 1)
#define FEATURE_TYPE_EXECUTE(feature, engine) FeatureType##feature##On##engine

class VpInterface;

class Policy
{
public:
    Policy(VpInterface &vpInterface);
    virtual ~Policy();
    MOS_STATUS CreateHwFilter(SwFilterPipe &subSwFilterPipe, HwFilter *&pFilter);
    MOS_STATUS Initialize();
    //!
    //! \brief    Check whether VEBOX-SFC Format Supported
    //! \details  Check whether VEBOX-SFC Format Supported.
    //! \param    inputFormat
    //!           [in] Format of Input Frame
    //! \param    outputFormat
    //!           [in] Format of Output Frame
    //! \return   bool
    //!           Return true if supported, otherwise failed
    //!
    bool IsVeboxSfcFormatSupported(MOS_FORMAT formatInput, MOS_FORMAT formatOutput);

    std::vector<FeatureType> &GetFeatureRegistered()
    {
        return m_featurePool;
    }
    
    virtual MOS_STATUS UpdateVpHwCapsBasedOnSku(VP_HW_CAPS &vpHwCaps);

protected:
    virtual MOS_STATUS RegisterFeatures();
    virtual void UnregisterFeatures();
    virtual MOS_STATUS RegisterFcFeatures();
    virtual MOS_STATUS GetExecutionCapsForSingleFeature(FeatureType featureType, SwFilterSubPipe& swFilterPipe, VP_EngineEntry& engineCapsCombined);
    virtual MOS_STATUS UpdateExeCaps(SwFilter* feature, VP_EXECUTE_CAPS& caps, EngineType Type);
    virtual MOS_STATUS UpdateCGCMode(SwFilter* feature, VP_EXECUTE_CAPS& caps, EngineType Type);
    virtual MOS_STATUS BuildVeboxSecureFilters(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);
    virtual MOS_STATUS GetExecutionCapsForAiSwFilterSubPipe(SwFilterAiBase *swAiFilter, VP_EngineEntry &engineCapsCombined);

    MOS_STATUS UpdateExecuteEngineCapsForHDR(SwFilterPipe &swFilterPipe, VP_EngineEntry &engineCapsCombinedAllPipes);
    MOS_STATUS UpdateExecuteEngineCapsForCrossPipeFeatures(SwFilterPipe &swFilterPipe, VP_EngineEntry &engineCapsCombinedAllPipes);
    MOS_STATUS         BuildExecutionEngines(SwFilterPipe &swFilterPipe, bool isInputPipe, uint32_t index, bool isAiPipe, VP_EngineEntry &engineCapsCombinedAllPipes);
    MOS_STATUS GetHwFilterParam(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params);
    MOS_STATUS ReleaseHwFilterParam(HW_FILTER_PARAMS &params);
    MOS_STATUS InitExecuteCaps(VP_EXECUTE_CAPS &caps, VP_EngineEntry &engineCapsInputPipe, VP_EngineEntry &engineCapsOutputPipe);
    MOS_STATUS GetExecuteCaps(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params);
    MOS_STATUS Update3DLutoutputColorAndFormat(FeatureParamCsc *cscParams, FeatureParamHdr *hdrParams, MOS_FORMAT Format, VPHAL_CSPACE CSpace);
    MOS_STATUS GetCSCExecutionCapsHdr(SwFilter *hdr, SwFilter *csc);
    MOS_STATUS GetCSCExecutionCapsDi(SwFilter* feature);
    MOS_STATUS GetCSCExecutionCapsBT2020ToRGB(SwFilter *cgc, SwFilter *csc);
    MOS_STATUS GetCSCExecutionCaps(SwFilter* feature, bool isCamPipeWithBayerInput);
    bool IsSfcSupported(MOS_FORMAT format);
    MOS_STATUS GetScalingExecutionCaps(SwFilter* feature, bool isDIEnabled);
    MOS_STATUS GetScalingExecutionCaps(SwFilter *feature, bool isHdrEnabled, bool isDIEnabled);
    MOS_STATUS GetScalingExecutionCapsHdr(SwFilter *feature);
    bool IsSfcRotationSupported(FeatureParamRotMir *rotationParams);
    MOS_STATUS GetRotationExecutionCaps(SwFilter* feature);
    virtual MOS_STATUS GetDenoiseExecutionCaps(SwFilter* feature);
    MOS_STATUS GetSteExecutionCaps(SwFilter* feature);
    MOS_STATUS GetTccExecutionCaps(SwFilter* feature);
    MOS_STATUS GetProcampExecutionCaps(SwFilter* feature);
    MOS_STATUS GetHdrExecutionCaps(SwFilter *feature);
    MOS_STATUS GetExecutionCaps(SwFilter* feature);
    MOS_STATUS GetDeinterlaceExecutionCaps(SwFilter* feature, bool is2PassScalingNeeded);
    MOS_STATUS GetColorFillExecutionCaps(SwFilter* feature);
    MOS_STATUS GetAlphaExecutionCaps(SwFilter* feature);
    MOS_STATUS GetLumakeyExecutionCaps(SwFilter* feature);
    MOS_STATUS GetBlendingExecutionCaps(SwFilter* feature);
    MOS_STATUS GetCgcExecutionCaps(SwFilter* feature);

    MOS_STATUS BuildExecuteCaps(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS &caps, VP_EngineEntry &engineCapsInputPipe, VP_EngineEntry &engineCapsOutputPipe,
                                bool &isSingleSubPipe, uint32_t &selectedPipeIndex);
    MOS_STATUS BypassVeboxFeatures(SwFilterSubPipe *featureSubPipe, VP_EngineEntry &engineCaps);
    MOS_STATUS GetInputPipeEngineCaps(SwFilterPipe& featurePipe, VP_EngineEntry &engineCapsInputPipe,
                                    SwFilterSubPipe *&singlePipeSelected, bool &isSingleSubPipe, uint32_t &selectedPipeIndex);

    // inputPipeSelected != nullptr for single input pipe case, otherwise, it's multi-input pipe case.
    MOS_STATUS GetOutputPipeEngineCaps(SwFilterPipe& featurePipe, VP_EngineEntry &engineCaps, SwFilterSubPipe *inputPipeSelected);

    MOS_STATUS UpdateFeatureTypeWithEngine(std::vector<int> &layerIndexes, SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps,
                                        bool isolatedFeatureSelected, bool outputPipeNeeded);
    MOS_STATUS UpdateFeatureTypeWithEngineSingleLayer(SwFilterSubPipe *featureSubPipe, VP_EXECUTE_CAPS& caps, bool isolatedFeatureSelected);
    virtual MOS_STATUS LayerSelectForProcess(std::vector<int> &layerIndexes, SwFilterPipe& featurePipe, bool isSingleSubPipe, uint32_t pipeIndex, VP_EXECUTE_CAPS& caps);

    MOS_STATUS UpdateFeaturePipe(SwFilterPipe &featurePipe, uint32_t pipeIndex, SwFilterPipe &executedFilters, uint32_t executePipeIndex,
                                bool isInputPipe, VP_EXECUTE_CAPS& caps);

    MOS_STATUS UpdateFeaturePipeSingleLayer(SwFilterPipe &featurePipe, uint32_t pipeIndex, SwFilterPipe &executedFilters, uint32_t executePipeIndex, VP_EXECUTE_CAPS& caps);
    MOS_STATUS UpdateFeatureOutputPipe(std::vector<int> &layerIndexes, SwFilterPipe &featurePipe, SwFilterPipe &executedFilters, VP_EXECUTE_CAPS& caps);
    MOS_STATUS BuildFilters(SwFilterPipe& subSwFilterPipe, HW_FILTER_PARAMS& params);
    MOS_STATUS BuildExecuteFilter(SwFilterPipe& swFilterPipe, std::vector<int> &layerIndexes, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);
    MOS_STATUS BuildExecuteHwFilter(VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);
    MOS_STATUS SetupExecuteFilter(SwFilterPipe& featurePipe, std::vector<int> &layerIndexes, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);
    MOS_STATUS SetupFilterResource(SwFilterPipe& featurePipe, std::vector<int> &layerIndexes, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);

    bool IsDemosaicValidOutputFormat(MOS_FORMAT format);
    bool IsAlphaSettingSupportedBySfc(MOS_FORMAT formatInput, MOS_FORMAT formatOutput, PVPHAL_ALPHA_PARAMS compAlpha);
    bool IsAlphaSettingSupportedByVebox(MOS_FORMAT formatInput, MOS_FORMAT formatOutput, PVPHAL_ALPHA_PARAMS compAlpha);
    bool IsIsolateFeatureOutputPipeNeeded(SwFilterSubPipe *featureSubPipe, SwFilter *swFilter);

    virtual MOS_STATUS AddFiltersBasedOnCaps(
        SwFilterPipe& featurePipe,
        uint32_t pipeIndex,
        VP_EXECUTE_CAPS& caps,
        SwFilterPipe& executedFilters,
        uint32_t executedPipeIndex);
    MOS_STATUS AddNewFilterOnVebox(
        SwFilterPipe& featurePipe,
        uint32_t pipeIndex,
        VP_EXECUTE_CAPS& caps,
        SwFilterPipe& executedFilters,
        uint32_t executedPipeIndex,
        FeatureType featureType);
    virtual MOS_STATUS GetCscParamsOnCaps(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS &caps, FeatureParamCsc &cscParams);
    virtual MOS_STATUS GetDnParamsOnCaps(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS &caps, FeatureParamDenoise &dnParams);

    MOS_STATUS AssignExecuteResource(VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params);

    virtual bool IsExcludedFeatureForHdr(FeatureType feature);

    virtual MOS_STATUS FilterFeatureCombination(SwFilterPipe &pipe, bool isInputPipe, uint32_t index, VP_EngineEntry &engineCapsCombined, VP_EngineEntry &engineCapsCombinedAllPipes);
    virtual MOS_STATUS FilterFeatureCombinationForHDRKernel(SwFilterSubPipe *pipe);
    MOS_STATUS AddCommonFilters(SwFilterSubPipe &swFilterSubPipe, VP_SURFACE *input, VP_SURFACE *outputs);

    virtual bool IsVeboxSecurePathEnabled(SwFilterPipe& subSwFilterPipe, VP_EXECUTE_CAPS& caps)
    {
        return false;
    }

    virtual MOS_STATUS UpdateSecureExecuteResource(SwFilterPipe& featurePipe, VP_EXECUTE_CAPS& caps, HW_FILTER_PARAMS& params)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual bool IsSecureResourceNeeded(VP_EXECUTE_CAPS& caps)
    {
        VP_FUNC_CALL();

        return false;
    }
    virtual bool Is3DLutKernelSupported()
    {
        return true;
    }

    virtual bool IsHDR33LutSizeSupported()
    {
        return m_hwCaps.m_rules.isHDR33LutSizeEnabled;
    }

    std::map<FeatureType, PolicyFeatureHandler*> m_VeboxSfcFeatureHandlers;
    std::map<FeatureType, PolicyFeatureHandler*> m_RenderFeatureHandlers;
    std::vector<FeatureType> m_featurePool;

    VpInterface         &m_vpInterface;
    VP_HW_CAPS          m_hwCaps = {};
    bool                m_initialized = false;

    // HDR 3DLut Parameters
    uint32_t            m_savedMaxDLL   = 1000;
    uint32_t            m_savedMaxCLL   = 4000;
    VPHAL_HDR_MODE      m_savedHdrMode  = VPHAL_HDR_MODE_NONE;

    //!
    //! \brief    Check whether Alpha Supported
    //! \details  Check whether Alpha Supported.
    //! \param    scalingParams
    //!           [in] Params of Scaling
    //! \return   bool
    //!           Return true if enabled, otherwise failed
    //!
    virtual bool IsColorfillEnabled(FeatureParamScaling *scalingParams);
    //!
    //! \brief    Check whether Colorfill Supported
    //! \details  Check whether Colorfill Supported.
    //! \param    scalingParams
    //!           [in] Params of Scaling
    //! \return   bool
    //!           Return true if enabled, otherwise failed
    //!
    virtual bool IsAlphaEnabled(FeatureParamScaling *scalingParams);

    virtual bool IsHDRfilterExist(SwFilterSubPipe *inputPipe)
    {
        if (inputPipe)
        {
            SwFilter *feature = (SwFilter *)inputPipe->GetSwFilter(FeatureType(FeatureTypeHdr));
            return feature != nullptr;
        }
        return false;
    }

    void PrintFeatureExecutionCaps(const char *name, VP_EngineEntry &engineCaps);

MEDIA_CLASS_DEFINE_END(vp__Policy)
};

}
#endif // !__POLICY_H__