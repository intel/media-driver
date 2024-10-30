/*
* Copyright (c) 2019-2024, Intel Corporation
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
//! \file     sw_filter.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __SW_FILTER_H__
#define __SW_FILTER_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_render_common.h"
#include <vector>
#include "media_sfc_interface.h"
#include "surface_type.h"

namespace vp
{
class VpInterface;
class SwFilterSubPipe;

#define FEATURE_TYPE_ENGINE_BITS_SFC        0x20
#define FEATURE_TYPE_ENGINE_BITS_VEBOX      0x40
#define FEATURE_TYPE_ENGINE_BITS_RENDER     0x80

#define FEATURE_TYPE_ENGINE_BITS_SUB_STEP   0x01

#define IS_FEATURE_TYPE_ON_SFC(type)        ((type)&FEATURE_TYPE_ENGINE_BITS_SFC)
#define IS_FEATURE_TYPE_ON_VEBOX(type)      ((type)&FEATURE_TYPE_ENGINE_BITS_VEBOX)
#define IS_FEATURE_TYPE_ON_VEBOX_SFC(type)  (IS_FEATURE_TYPE_ON_SFC(type) || IS_FEATURE_TYPE_ON_VEBOX(type))
#define IS_FEATURE_TYPE_ON_RENDER(type)     ((type)&FEATURE_TYPE_ENGINE_BITS_RENDER)

#define FEATURE_TYPE_MASK                   0xffffff00
#define FEATURE_TYPE_ENGINE_ASSIGNED(feature) (((feature)&FEATURE_TYPE_MASK) != (feature))

#define VPHAL_MAX_HDR_INPUT_LAYER           8
#define VPHAL_MAX_HDR_OUTPUT_LAYER          1
#define VPHAL_HDR_BTINDEX_LAYER0            16
#define VPHAL_HDR_BTINDEX_PER_LAYER0        5
#define VPHAL_HDR_BTINDEX_RENDERTARGET      56
#define VPHAL_HDR_BTINDEX_PER_TARGET        3
#define VPHAL_HDR_SAMPLER8X8_TABLE_NUM      2
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof(a[0]))

enum FeatureType
{
    FeatureTypeInvalid              = 0,
    FeatureTypeCsc                  = 0x100,
    FeatureTypeCscOnSfc             = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeCscOnVebox           = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeCscOnRender          = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeRotMir               = 0x200,
    FeatureTypeRotMirOnSfc          = FeatureTypeRotMir | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeRotMirOnRender       = FeatureTypeRotMir | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeScaling              = 0x300,
    FeatureTypeScalingOnSfc         = FeatureTypeScaling | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeScalingOnRender      = FeatureTypeScaling | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeDn                   = 0x400,
    FeatureTypeDnOnVebox            = FeatureTypeDn | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeDnHVSCalOnRender     = FeatureTypeDn | FEATURE_TYPE_ENGINE_BITS_RENDER | FEATURE_TYPE_ENGINE_BITS_SUB_STEP,
    FeatureTypeDi                   = 0x500,
    FeatureTypeDiOnVebox            = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeDiOnRender           = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeSte                  = 0x600,
    FeatureTypeSteOnVebox           = FeatureTypeSte | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeVeboxUpdate          = 0x700,
    FeatureTypeVeboxUpdateOnRender  = FeatureTypeVeboxUpdate | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeTcc                  = 0x800,
    FeatureTypeTccOnVebox           = FeatureTypeTcc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeProcamp              = 0x900,
    FeatureTypeProcampOnVebox       = FeatureTypeProcamp | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeProcampOnRender      = FeatureTypeProcamp | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeCgc                  = 0xA00,
    FeatureTypeCgcOnVebox           = FeatureTypeCgc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeHdr                  = 0xB00,
    FeatureTypeHdrOnVebox           = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeHdr3DLutCalOnRender  = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_RENDER | FEATURE_TYPE_ENGINE_BITS_SUB_STEP,
    FeatureTypeHdrOnRender          = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeSecureCopy           = 0xC00,
    FeatureTypeSecureCopyOnRender   = FeatureTypeSecureCopy | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeGamutExt             = 0xD00,
    FeatureTypeFc                   = 0xE00,
    FeatureTypeFcOnRender           = FeatureTypeFc | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeLumakey              = 0xF00,
    FeatureTypeLumakeyOnRender      = FeatureTypeLumakey | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeBlending             = 0x1000,
    FeatureTypeBlendingOnRender     = FeatureTypeBlending | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeColorFill            = 0x1100,
    FeatureTypeColorFillOnSfc       = FeatureTypeColorFill | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeColorFillOnRender    = FeatureTypeColorFill | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeAlpha                = 0x1200,
    FeatureTypeAlphaOnSfc           = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeAlphaOnVebox         = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeAlphaOnRender        = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_RENDER,
    NumOfFeatureTypeBase,

#ifdef _MEDIA_RESERVED
#include "feature_type_ext.h"
#endif

    NumOfFeatureType
};

enum RenderTargetType
{
    RenderTargetTypeInvalid = 0,
    RenderTargetTypeSurface,
    RenderTargetTypeParameter
};

using  VP_SURFACE_GROUP = std::map<SurfaceType, VP_SURFACE*>;

enum OutMode
{
    OUT_DISABLED = 0,
    OUT_TO_TARGET,
    OUT_TO_NEXTPASS
};

struct REMOVE_BB_SETTING
{
    bool     isRemoveBB    = false;
    bool     isKeepMaxBlob = false;
    uint32_t index                          = 0;
    uint32_t height                         = 0;
    uint32_t width                          = 0;
    uint32_t size                           = 0;
    uint16_t inputActiveRegionWidth         = 0;
    uint16_t inputActiveRegionHeight        = 0;
    uint8_t *removeBlobLinearAddressAligned = 0;
};

struct MOTIONLESS_SETTING
{
    bool     isEnable        = false;
    bool     isSkipDetection = false;
    bool     isInfer         = false;
    bool     isFirstConv     = false;
    bool     isMotion        = false;
    bool     isResUpdate     = false;
    bool     forceDisable    = false;
    uint32_t width           = 0;
    uint32_t height          = 0;
};

struct COLOR_BALANCE_SETTING
{
    bool     isEnable                 = false;
    uint32_t index                    = 0;
    uint32_t skipThreshold            = 0;
    uint16_t inputActiveRegionWidth   = 0;
    uint16_t inputActiveRegionHeight  = 0;
    uint32_t pitch                    = 0;
    uint32_t mPitch                   = 0;
    uint8_t  downScaleFactor          = 1;
    uint8_t  colorTemperature         = 0;
    uint8_t *cbLinearAddressAligned   = 0;
    double   backgroundWhiteMatrix[3] = {1, 1, 1};
};

struct VP_POSTPROCESS_SURFACE
{
    REMOVE_BB_SETTING  removeBBSetting;
    MOTIONLESS_SETTING motionlessSetting;
    COLOR_BALANCE_SETTING colorBalanceSetting;
};

struct VP_SURFACE_SETTING
{
    VP_SURFACE_GROUP    surfGroup;
    bool                isPastHistogramValid       = false;
    uint32_t            imageWidthOfPastHistogram  = 0;
    uint32_t            imageHeightOfPastHistogram = 0;
    uint32_t            dwVeboxPerBlockStatisticsHeight = 0;
    uint32_t            dwVeboxPerBlockStatisticsWidth  = 0;
    uint32_t            aggregateBlockSize              = 0;
    bool                laceLutValid                    = false;
    bool                updateGlobalToneMappingCurveLUTSurface = false;
    bool                updateWeitCoefSurface                  = false;
    bool                dumpLaceSurface                        = false;
    bool                dumpPreSurface                         = false;
    bool                dumpPostSurface                        = false;
    VP_POSTPROCESS_SURFACE postProcessSurface                  = {};
    const uint16_t     *pHDRStageConfigTable                   = nullptr;
    bool                coeffAllocated                         = false;
    bool                OETF1DLUTAllocated                     = false;
    bool                Cri3DLUTAllocated                      = false;

    void Clean()
    {
        surfGroup.clear();
        isPastHistogramValid        = false;
        imageWidthOfPastHistogram   = 0;
        imageHeightOfPastHistogram  = 0;
        dwVeboxPerBlockStatisticsHeight = 0;
        dwVeboxPerBlockStatisticsWidth  = 0;
        aggregateBlockSize              = 0;
        laceLutValid                    = false;
        updateGlobalToneMappingCurveLUTSurface = true;
        updateWeitCoefSurface                  = true;
        dumpLaceSurface                        = false;
        dumpPreSurface                         = false;
        dumpPostSurface                        = false;
        postProcessSurface.removeBBSetting     = {};
        postProcessSurface.motionlessSetting   = {};
        postProcessSurface.colorBalanceSetting = {};
        pHDRStageConfigTable                   = nullptr;
        coeffAllocated                         = false;
        OETF1DLUTAllocated                     = false;
        Cri3DLUTAllocated                      = false;
    }
};

#define FEATURE_TYPE_MASK   0xffffff00
#define FEATURE_TYPE_ENGINE_ASSIGNED(feature) (((feature)&FEATURE_TYPE_MASK) != (feature))

inline bool operator==(FeatureType a, FeatureType b)
{
    return (int)a == (int)b || (int)(a & FEATURE_TYPE_MASK) == (int)b || (int)a == (int)(FEATURE_TYPE_MASK & b);
}

inline bool operator!=(FeatureType a, FeatureType b)
{
    return !(a == b);
}

inline bool operator<(FeatureType a, FeatureType b)
{
    return a != b && (int)a < (int)b;
}

#define RECT_ROTATE(rcOut, rcIn)        \
{                                       \
    (rcOut).left    = (rcIn).top;       \
    (rcOut).right   = (rcIn).bottom;    \
    (rcOut).top     = (rcIn).left;      \
    (rcOut).bottom  = (rcIn).right;     \
}

struct FeatureParam
{
    FeatureType type         = FeatureTypeInvalid;
    MOS_FORMAT  formatInput  = Format_None;
    MOS_FORMAT  formatOutput = Format_None;
};

enum FeatureCategory
{
    FeatureCategoryBasic    = 0,
    FeatureCategoryIsolated,
    FeatureCategoryFC,
};

class SwFilterSet;

class SwFilter
{
public:
    SwFilter(VpInterface &vpInterface, FeatureType type);
    virtual ~SwFilter();
    virtual MOS_STATUS Clean()
    {
        MOS_ZeroMemory(&m_EngineCaps, sizeof(m_EngineCaps));
        m_noNeedUpdate = false;
        m_isInExePipe = false;
        return MOS_STATUS_SUCCESS;
    }
    virtual FeatureType GetFeatureType()
    {
        return m_type;
    }
    virtual bool IsEngineAssigned()
    {
        return m_type != (m_type & FEATURE_TYPE_MASK);
    }
    virtual RenderTargetType GetRenderTargetType()
    {
        return m_renderTargetType;
    }
    bool IsFeatureEnabled(VP_EXECUTE_CAPS caps)
    {
        return m_EngineCaps.bEnabled && (m_EngineCaps.SfcNeeded && caps.bSFC ||
            m_EngineCaps.VeboxNeeded && caps.bVebox || m_EngineCaps.RenderNeeded && caps.bRender);
    }
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool bInputSurf, int surfIndex) = 0;
    virtual MOS_STATUS Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual MOS_STATUS Configure(SwFilter& swFilter, VP_EXECUTE_CAPS caps)
    {
        return MOS_STATUS_UNIMPLEMENTED;
    }

    virtual SwFilter *Clone() = 0;
    virtual bool operator == (class SwFilter&) = 0;
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe) = 0;
    virtual MOS_STATUS SetFeatureType(FeatureType type);
    virtual MOS_STATUS ResetFeatureType();
    virtual MOS_STATUS SetRenderTargetType(RenderTargetType type);
    SwFilter* CreateSwFilter(FeatureType type);
    void DestroySwFilter(SwFilter* p);

    void SetLocation(SwFilterSet *swFilterSet)
    {
        m_location = swFilterSet;
    }

    SwFilterSet *GetLocation()
    {
        return m_location;
    }

    VpInterface& GetVpInterface()
    {
        return m_vpInterface;
    }

    VP_EngineEntry& GetFilterEngineCaps()
    {
        return m_EngineCaps;
    }

    // For some feature in output pipe, enable or disable depends on other feature status of input pipe.
    // If singleInputPipeSelected != nullptr, means single input pipe inuse, otherwise, multi-input pipe in use.
    virtual VP_EngineEntry GetCombinedFilterEngineCaps(SwFilterSubPipe *singleInputPipeSelected)
    {
        return m_EngineCaps;
    }

    // The child class need to implement SetResourceAssignmentHint only when any feature
    // parameters will affect the resource assignment.
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        return MOS_STATUS_SUCCESS;
    }

    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        return MOS_STATUS_SUCCESS;
    }

    void SetExePipeFlag(bool isInExePipe)
    {
        m_isInExePipe = isInExePipe;
    }

    VP_MHWINTERFACE* GetHwInterface();

protected:
    VpInterface &m_vpInterface;
    FeatureType m_type = FeatureTypeInvalid;
    // SwFilterSet current swFilter belongs to.
    SwFilterSet *m_location = nullptr;
    VP_EngineEntry  m_EngineCaps = {};
    bool m_noNeedUpdate = false;
    RenderTargetType m_renderTargetType = RenderTargetTypeSurface;
    bool m_isInExePipe = false;

MEDIA_CLASS_DEFINE_END(vp__SwFilter)
};

struct FeatureParamCsc : public FeatureParam
{
    struct CSC_PARAMS
    {
        VPHAL_CSPACE    colorSpace      = CSpace_None;
        uint32_t        chromaSiting    = 0;
        MOS_TILE_MODE_GMM tileMode      = MOS_TILE_4_GMM;
        bool operator == (const struct CSC_PARAMS &b)
        {
            return colorSpace == b.colorSpace && chromaSiting == b.chromaSiting;
        }
    };
    CSC_PARAMS          input           = {};
    CSC_PARAMS          output          = {};
    PVPHAL_IEF_PARAMS   pIEFParams      = nullptr;
    PVPHAL_ALPHA_PARAMS pAlphaParams    = nullptr;
    MOS_FORMAT          formatforCUS    = Format_None;            //!< Only valid when formatforCUS != Format_None for Chromaupsampling. To be cleared in SwFilterCsc::Configure
    FeatureParamCsc     *next           = nullptr;                //!< pointe to new/next generated CSC params
    bool                isFullRgbG10P709 = false;                  //!< whether output colorspace is DXGI_COLOR_SPACE_RGB_FULL_G10_NONE_P709
};

class SwFilterCsc : public SwFilter
{
public:
    SwFilterCsc(VpInterface &vpInterface);
    virtual ~SwFilterCsc();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps);
    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params);
    virtual MOS_STATUS Configure(FeatureParamCsc &params);
    virtual FeatureParamCsc &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetFeatureType(FeatureType type);
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG5(MT_VP_FEATURE_GRAPH_SWFILTERCSC, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_INPUTCOLORSPACE, m_Params.input.colorSpace, MT_VP_FEATURE_GRAPH_FILTER_OUTPUTCOLORSPACE, m_Params.output.colorSpace,
                MT_VP_FEATURE_GRAPH_FILTER_INPUTFORMAT, m_Params.formatInput, MT_VP_FEATURE_GRAPH_FILTER_OUTPUTFORMAT, m_Params.formatOutput, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterCsc: inputcolorSpace %d, outputcolorSpace, %d, formatInput %d, formatOutput %d, FeatureType %d", m_Params.input.colorSpace, m_Params.output.colorSpace,
                                m_Params.formatInput, m_Params.formatOutput, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamCsc m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterCsc)
};

struct FeatureParamScaling : public FeatureParam
{
    struct SCALING_PARAMS
    {
        uint32_t                dwWidth  = 0;
        uint32_t                dwHeight = 0;
        uint32_t                dwPitch  = 0;
        RECT                    rcSrc    = {0, 0, 0, 0};
        RECT                    rcDst    = {0, 0, 0, 0};  //!< Input dst rect without rotate being applied.
        RECT                    rcMaxSrc = {0, 0, 0, 0};
        VPHAL_SAMPLE_TYPE       sampleType = SAMPLE_PROGRESSIVE;
        MOS_TILE_MODE_GMM       tileMode   = MOS_TILE_4_GMM;
        bool operator == (struct SCALING_PARAMS &b)
        {
            // no use sizeof(SCALING_PARAMS) to avoid undefined padding data being used.
            return 0 == memcmp(this, &b, (uint64_t)(&tileMode) - (uint64_t)(this) + sizeof(tileMode));
        }
    };

    // Parameters maintained by scaling feature parameters
    SCALING_PARAMS              input       = {};
    SCALING_PARAMS              output      = {};
    bool                        isPrimary   = false;
    VPHAL_SCALING_MODE          scalingMode = VPHAL_SCALING_NEAREST;
    VPHAL_SCALING_PREFERENCE    scalingPreference  = VPHAL_SCALING_PREFER_SFC;  //!< DDI indicate Scaling preference
    bool                        bDirectionalScalar = false;     //!< Vebox Directional Scalar
    bool                        bTargetRectangle   = false;     // Target rectangle enabled
    PVPHAL_COLORFILL_PARAMS     pColorFillParams = nullptr;     //!< ColorFill - BG only
    PVPHAL_ALPHA_PARAMS         pCompAlpha       = nullptr;     //!< Alpha for composited surfaces
    VPHAL_ISCALING_TYPE         interlacedScalingType = ISCALING_NONE;

    // Parameters maintained by other feature parameters.
    struct {
        VPHAL_CSPACE colorSpaceOutput = CSpace_None;
    } csc;

    struct {
        bool                    rotationNeeded = false;                 //!< Whether rotate SwFilter exists on SwFilterPipe.
    } rotation;

    bool operator == (struct FeatureParamScaling &b)
    {
        return formatInput          == b.formatInput            &&
            formatOutput            == b.formatOutput           &&
            input                   == b.input                  &&
            output                  == b.output                 &&
            isPrimary               == b.isPrimary              &&
            scalingMode             == b.scalingMode            &&
            scalingPreference       == b.scalingPreference      &&
            bDirectionalScalar      == b.bDirectionalScalar     &&
            bTargetRectangle        == b.bTargetRectangle       &&
            interlacedScalingType   == b.interlacedScalingType  &&
            csc.colorSpaceOutput    == b.csc.colorSpaceOutput   &&
            rotation.rotationNeeded == b.rotation.rotationNeeded &&
            (nullptr == pColorFillParams    && nullptr == b.pColorFillParams ||
            nullptr != pColorFillParams     && nullptr != b.pColorFillParams &&
            0 == memcmp(pColorFillParams, b.pColorFillParams, sizeof(VPHAL_COLORFILL_PARAMS))) &&
            (nullptr == pCompAlpha          && nullptr == b.pCompAlpha ||
            nullptr != pCompAlpha           && nullptr != b.pCompAlpha &&
            0 == memcmp(pCompAlpha, b.pCompAlpha, sizeof(VPHAL_ALPHA_PARAMS)));
    }

    FeatureParamScaling        *next = nullptr;                           //!< pointe to new/next generated scaling params
};

class SwFilterScaling : public SwFilter
{
public:
    SwFilterScaling(VpInterface &vpInterface);
    virtual ~SwFilterScaling();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params);
    virtual MOS_STATUS Configure(PVP_SURFACE surfInput, PVP_SURFACE surfOutput, VP_EXECUTE_CAPS caps);
    virtual FeatureParamScaling &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.isIScalingTypeNone = ISCALING_NONE == m_Params.interlacedScalingType;
        hint.isFieldWeaving     = ISCALING_FIELD_TO_INTERLEAVED == m_Params.interlacedScalingType;
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG7(MT_VP_FEATURE_GRAPH_SWFILTERSCALING, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_INPUTHEIGHT, m_Params.input.dwHeight, MT_VP_FEATURE_GRAPH_FILTER_INPUTWIDTH, m_Params.input.dwWidth,
                MT_VP_FEATURE_GRAPH_FILTER_INPUTTILEMODE, m_Params.input.tileMode, MT_VP_FEATURE_GRAPH_FILTER_OUTPUTHEIGHT, m_Params.output.dwHeight, MT_VP_FEATURE_GRAPH_FILTER_OUTPUTWIDTH,
                m_Params.output.dwWidth, MT_VP_FEATURE_GRAPH_FILTER_OUTPUTTILEMODE, m_Params.output.tileMode, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterScaling: input_dwHeight %d, input_dwWidth, %d, input_tileMode %d, output_dwHeight %d, output_dwWidth %d, output_tileMode %d, interlacedScalingType %d, \
                                 scalingMode %d, FeatureType %d", m_Params.input.dwHeight, m_Params.input.dwWidth, m_Params.input.tileMode, m_Params.output.dwHeight, m_Params.output.dwWidth,
                                 m_Params.output.tileMode, m_Params.interlacedScalingType, m_Params.scalingMode, GetFeatureType());

        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamScaling m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterScaling)
};

struct FeatureParamRotMir : public FeatureParam
{
    // Parameters maintained by rotation feature parameters
    VPHAL_ROTATION rotation = VPHAL_ROTATION_IDENTITY;

    // Parameters maintained by other feature parameters.
    struct {
        MOS_TILE_TYPE tileOutput = MOS_TILE_X;
    } surfInfo;

    bool operator == (const struct FeatureParamRotMir &b)
    {
        return rotation == b.rotation &&
            surfInfo.tileOutput == b.surfInfo.tileOutput;
    }
};

class SwFilterRotMir : public SwFilter
{
public:
    SwFilterRotMir(VpInterface &vpInterface);
    virtual ~SwFilterRotMir();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params);
    virtual FeatureParamRotMir &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG2(MT_VP_FEATURE_GRAPH_SWFILTERROTMIR, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_ROTATION, m_Params.rotation, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterRotMir: rotation %d, FeatureType %d", m_Params.rotation, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamRotMir m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterRotMir)
};

enum DN_STAGE
{
    DN_STAGE_DEFAULT = 0,
    DN_STAGE_HVS_KERNEL,
    DN_STAGE_VEBOX_HVS_UPDATE,
    DN_STAGE_VEBOX_HVS_NO_UPDATE,
};

struct FeatureParamDenoise : public FeatureParam
{
    VPHAL_SAMPLE_TYPE    sampleTypeInput      = SAMPLE_PROGRESSIVE;
    VPHAL_DENOISE_PARAMS denoiseParams        = {};
    uint32_t             widthAlignUnitInput  = 0;
    uint32_t             heightAlignUnitInput = 0;
    uint32_t             heightInput          = 0;
    bool                 secureDnNeeded       = false;
    DN_STAGE             stage                = DN_STAGE_DEFAULT;
    uint32_t             srcBottom            = 0;
    bool                 operator==(const struct FeatureParamDenoise &b)
    {
        return sampleTypeInput     == b.sampleTypeInput &&
               denoiseParams       == b.denoiseParams   &&
               widthAlignUnitInput == b.widthAlignUnitInput &&
               heightAlignUnitInput == b.heightAlignUnitInput &&
               MOS_MIN(heightInput, srcBottom) == MOS_MIN(b.heightInput, srcBottom) &&
               secureDnNeeded      == b.secureDnNeeded &&
               stage               == b.stage;
    }
};

class SwFilterDenoise : public SwFilter
{
public:
    SwFilterDenoise(VpInterface& vpInterface);
    virtual ~SwFilterDenoise();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(FeatureParamDenoise &params);
    virtual FeatureParamDenoise& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS           SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.isHVSTableNeeded = DN_STAGE_HVS_KERNEL == m_Params.stage ||
                                DN_STAGE_VEBOX_HVS_UPDATE == m_Params.stage;
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG7(MT_VP_FEATURE_GRAPH_SWFILTERDENOISE, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_CHROMADN, m_Params.denoiseParams.bEnableChroma, MT_VP_FEATURE_GRAPH_FILTER_LUMADN, m_Params.denoiseParams.bEnableLuma,
                MT_VP_FEATURE_GRAPH_FILTER_AUTODETECT, m_Params.denoiseParams.bAutoDetect, MT_VP_FEATURE_GRAPH_FILTER_HVSDN, m_Params.denoiseParams.bEnableHVSDenoise, MT_VP_FEATURE_GRAPH_FILTER_DNFACTOR,
                (int64_t)m_Params.denoiseParams.fDenoiseFactor, MT_VP_FEATURE_GRAPH_FILTER_SECUREDNNEED, m_Params.secureDnNeeded, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterDenoise: bEnableChroma %d, bEnableLuma %d, bAutoDetect %d, bEnableHVSDenoise %d, fDenoiseFactor %f, secureDnNeeded %d, FeatureType %d", m_Params.denoiseParams.bEnableChroma,
                                m_Params.denoiseParams.bEnableLuma, m_Params.denoiseParams.bAutoDetect, m_Params.denoiseParams.bEnableHVSDenoise, m_Params.denoiseParams.fDenoiseFactor, m_Params.secureDnNeeded, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamDenoise m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterDenoise)
};

struct FeatureParamDeinterlace : public FeatureParam
{
    VPHAL_SAMPLE_TYPE       sampleTypeInput      = SAMPLE_PROGRESSIVE;
    bool                    bHDContent           = false;
    PVPHAL_DI_PARAMS        diParams             = nullptr;
    bool                    bFmdExtraVariance    = false;     //!< Check if extra FMD variances need to be calculated
    bool                    bQueryVarianceEnable = false;     //!< Query variance enabled
    uint32_t                heightInput          = 0;
    RECT                    rcSrc                = {0, 0, 0, 0};
};

class SwFilterDeinterlace : public SwFilter
{
public:
    SwFilterDeinterlace(VpInterface& vpInterface);
    virtual ~SwFilterDeinterlace();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamDeinterlace& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.bDi        = 1;
        hint.b60fpsDi   = m_Params.diParams && !m_Params.diParams->bSingleField;
        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG4(MT_VP_FEATURE_GRAPH_SWFILTERDEINTERLACE, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_SAMPLETYPEINPUT, m_Params.sampleTypeInput,
                 MT_VP_FEATURE_GRAPH_FILTER_SINGLEFIELD, m_Params.diParams ? m_Params.diParams->bSingleField : -1, MT_VP_FEATURE_GRAPH_FILTER_DIMODE, m_Params.diParams ? m_Params.diParams->DIMode : -1,
                 MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterDeinterlace: sampleTypeInput %d, bSingleField %d, DIMode %d, FeatureType %d", m_Params.sampleTypeInput,
                                m_Params.diParams ? m_Params.diParams->bSingleField : -1, m_Params.diParams ? m_Params.diParams->DIMode : -1, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamDeinterlace m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterDeinterlace)
};

struct FeatureParamSte : public FeatureParam
{
    bool              bEnableSTE  = false;
    uint32_t          dwSTEFactor = 0;
    
    // For STD alone case
    bool              bEnableSTD  = false;
    VPHAL_STD_PARAMS  STDParam    = {};
};

class SwFilterSte : public SwFilter
{
public:
    SwFilterSte(VpInterface& vpInterface);
    virtual ~SwFilterSte();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamSte& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        MT_LOG1(MT_VP_FEATURE_GRAPH_SWFILTERSTD, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_STD_OUTPUT_ENABLE, m_Params.bEnableSTD);
        VP_PUBLIC_NORMALMESSAGE("STD Output Enable: %d", m_Params.bEnableSTD);
        if (m_Params.bEnableSTD)
        {
            // isSkinScoreDumpNeededForSTDonly for STD output to internal surface and copy to STDParam.
            // isSkinScoreOutputNeededForSTDOnly for STD output to output surface directly. It set internal
            // SkinScore surface to VeboxCurrentOutput and external output surface to VeboxSkinScore to
            // achieve it, instead of setting Output STD Decisions to 1.
            hint.isSkinScoreDumpNeededForSTDonly = !m_Params.STDParam.bOutputSkinScore;
            hint.isSkinScoreOutputNeededForSTDOnly = m_Params.STDParam.bOutputSkinScore;
            MT_LOG2(MT_VP_FEATURE_GRAPH_SWFILTERSTD, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_STD_OUTPUT_TO_STDPARAM, hint.isSkinScoreDumpNeededForSTDonly, MT_VP_FEATURE_GRAPH_FILTER_STD_OUTPUT_TO_OUTPUT_SURFACE, hint.isSkinScoreOutputNeededForSTDOnly);
            VP_PUBLIC_NORMALMESSAGE("STD Output isSkinScoreDumpNeededForSTDonly: %d, isSkinScoreOutputNeededForSTDOnly: %d", hint.isSkinScoreDumpNeededForSTDonly, hint.isSkinScoreOutputNeededForSTDOnly);
        }

        return MOS_STATUS_SUCCESS;
    }
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG3(MT_VP_FEATURE_GRAPH_SWFILTERSTE, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_STEFACTOR, m_Params.dwSTEFactor, MT_VP_FEATURE_GRAPH_FILTER_ENABLESTD, m_Params.bEnableSTD, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterSte: dwSTEFactor %d, bEnableSTD %d, FeatureType %d", m_Params.dwSTEFactor, m_Params.bEnableSTD, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamSte m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterSte)
};

struct FeatureParamTcc : public FeatureParam
{
    bool                bEnableTCC = false;
    uint8_t             Red        = 0;
    uint8_t             Green      = 0;
    uint8_t             Blue       = 0;
    uint8_t             Cyan       = 0;
    uint8_t             Magenta    = 0;
    uint8_t             Yellow     = 0;
};

class SwFilterTcc : public SwFilter
{
public:
    SwFilterTcc(VpInterface& vpInterface);
    virtual ~SwFilterTcc();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamTcc& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG4(MT_VP_FEATURE_GRAPH_SWFILTERTCC, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_TCCRED, m_Params.Red, MT_VP_FEATURE_GRAPH_FILTER_TCCGREEN, m_Params.Green, MT_VP_FEATURE_GRAPH_FILTER_TCCBLUE,
                m_Params.Blue, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterTcc: Red %d, Green %d, Blue %d, FeatureType %d", m_Params.Red, m_Params.Green, m_Params.Blue, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamTcc m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterTcc)
};

struct FeatureParamProcamp : public FeatureParam
{
    PVPHAL_PROCAMP_PARAMS procampParams = nullptr;
};

class SwFilterProcamp : public SwFilter
{
public:
    SwFilterProcamp(VpInterface& vpInterface);
    virtual ~SwFilterProcamp();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamProcamp& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG5(MT_VP_FEATURE_GRAPH_SWFILTERPROCAMP, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_BRIGHTNESS, m_Params.procampParams ? (int64_t)m_Params.procampParams->fBrightness : -1,
                MT_VP_FEATURE_GRAPH_FILTER_CONTRAST, m_Params.procampParams ? (int64_t)m_Params.procampParams->fContrast : -1, MT_VP_FEATURE_GRAPH_FILTER_HUE, m_Params.procampParams ? (int64_t)m_Params.procampParams->fHue : -1,
                MT_VP_FEATURE_GRAPH_FILTER_SATURATION, m_Params.procampParams ? (int64_t) m_Params.procampParams->fSaturation : -1, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterProcamp: fBrightness %f, fContrast %f, fHue %f, fSaturation %f, FeatureType %d", m_Params.procampParams ? m_Params.procampParams->fBrightness : -1,
                                m_Params.procampParams ? m_Params.procampParams->fContrast : -1, m_Params.procampParams ? m_Params.procampParams->fHue : -1,
                                m_Params.procampParams ? m_Params.procampParams->fSaturation : -1, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamProcamp m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterProcamp)
};

enum HDR_STAGE
{
    HDR_STAGE_DEFAULT = 0,
    HDR_STAGE_3DLUT_KERNEL,
    HDR_STAGE_VEBOX_3DLUT_UPDATE,
    HDR_STAGE_VEBOX_3DLUT_NO_UPDATE,
    HDR_STAGE_VEBOX_EXTERNAL_3DLUT,
};

//!
//! \brief Hdr stages enable flag
//!
typedef union _HDRStageEnables
{
    uint16_t value;
    struct
    {
        uint16_t PriorCSCEnable : 1;
        uint16_t EOTFEnable : 1;
        uint16_t CCMEnable : 1;
        uint16_t PWLFEnable : 1;
        uint16_t CCMExt1Enable : 1;
        uint16_t GamutClamp1Enable : 1;
        uint16_t CCMExt2Enable : 1;
        uint16_t GamutClamp2Enable : 1;
        uint16_t OETFEnable : 1;
        uint16_t PostCSCEnable : 1;
        uint16_t Reserved : 6;
    };
} HDRStageEnables, *PHDRStageEnables;

//!
//! Structure VPHAL_HDR_PARAMS
//! \brief High Dynamic Range parameters
//!
typedef struct _HDR_PARAMS
{
    VPHAL_HDR_EOTF_TYPE EOTF                 = VPHAL_HDR_EOTF_INVALID;    //!< Electronic-Optimal Transfer Function
    uint16_t display_primaries_x[3]          = {0};                       //!< Display Primaries X chromaticity coordinates
    uint16_t display_primaries_y[3]          = {0};                       //!< Display Primaries Y chromaticity coordinates
    uint16_t white_point_x                   = 0;                         //!< X Chromaticity coordinate of White Point
    uint16_t white_point_y                   = 0;                         //!< Y Chromaticity coordinate of White Point
    uint16_t max_display_mastering_luminance = 0;                         //!< The nominal maximum display luminance of the mastering display
    uint16_t min_display_mastering_luminance = 0;                         //!< The nominal minimum display luminance of the mastering display
    uint16_t MaxCLL                          = 0;                         //!< Max Content Light Level
    uint16_t MaxFALL                         = 0;                         //!< Max Frame Average Light Level
    bool     bAutoMode                       = false;                     //!< Hdr auto mode.
    bool     bPathKernel                     = false;                     //!< Hdr path config to use kernel
} HDR_PARAMS, *PHDR_PARAMS;

struct FeatureParamHdr : public FeatureParam
{
    uint32_t           uiMaxDisplayLum                                      = 0;                   //!< Maximum Display Luminance
    uint32_t           uiMaxContentLevelLum                                 = 0;                   //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE     hdrMode                                              = VPHAL_HDR_MODE_NONE;
    VPHAL_CSPACE       srcColorSpace                                        = CSpace_None;
    VPHAL_CSPACE       dstColorSpace                                        = CSpace_None;
    HDR_STAGE          stage                                                = HDR_STAGE_DEFAULT;
    uint32_t           widthInput                                           = 0;
    uint32_t           heightInput                                          = 0;
    uint32_t           lutSize                                              = 0;
    VPHAL_HDR_LUT_MODE lutMode                                              = VPHAL_HDR_LUT_MODE_NONE; //!< LUT Mode
    VPHAL_HDR_LUT_MODE globalLutMode                                        = VPHAL_HDR_LUT_MODE_NONE; //!< Global LUT mode control for debugging purpose
    bool               bGpuGenerate3DLUT                                    = false;               //!< Flag for per frame GPU generation of 3DLUT
    bool               is3DLutKernelOnly                                    = false;

    PVPHAL_COLORFILL_PARAMS pColorFillParams                     = nullptr;               //!< ColorFill - BG only
    bool                    bDisableAutoMode                     = false;                 //!< Force to disable Hdr auto mode tone mapping for debugging purpose
    uint32_t                uiSplitFramePortions                 = 1;                     //!< Split Frame flag
    bool                    bForceSplitFrame                     = false;
    bool                    bNeed3DSampler                       = false;                 //!< indicate whether 3D should neede by force considering AVS removal etc.
    bool                    isOclKernelEnabled                    = false;

    HDR_PARAMS srcHDRParams    = {};
    HDR_PARAMS targetHDRParams = {};
    PVPHAL_3DLUT_PARAMS external3DLutParams = nullptr;
};

class SwFilterHdr : public SwFilter
{
public:
    SwFilterHdr(VpInterface &vpInterface);
    virtual ~SwFilterHdr();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamHdr &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        hint.is3DLut2DNeeded = HDR_STAGE_3DLUT_KERNEL == m_Params.stage ||
                               HDR_STAGE_VEBOX_3DLUT_UPDATE == m_Params.stage;
        return MOS_STATUS_SUCCESS;
    }

    MOS_STATUS HdrIsInputFormatSupported(
        PVPHAL_SURFACE pSrcSurface,
        bool          *pbSupported);

    MOS_STATUS HdrIsOutputFormatSupported(
            PVPHAL_SURFACE pTargetSurface,
            bool          *pbSupported);
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG7(MT_VP_FEATURE_GRAPH_SWFILTERHDR, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_HDRMODE, m_Params.hdrMode, MT_VP_FEATURE_GRAPH_FILTER_GPUGENERATE3DLUT, m_Params.bGpuGenerate3DLUT,
                MT_VP_FEATURE_GRAPH_FILTER_INPUTFORMAT, m_Params.formatInput, MT_VP_FEATURE_GRAPH_FILTER_OUTPUTFORMAT, m_Params.formatOutput, MT_VP_FEATURE_GRAPH_FILTER_INPUTCOLORSPACE,
                m_Params.srcColorSpace, MT_VP_FEATURE_GRAPH_FILTER_OUTPUTCOLORSPACE, m_Params.dstColorSpace, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterHdr: hdrMode %d, lutMode %d, bGpuGenerate3DLUT %d, formatInput %d, formatOutput %d, srcColorSpace %d, dstColorSpace %d, FeatureType %d", m_Params.hdrMode,
                                m_Params.lutMode, m_Params.bGpuGenerate3DLUT, m_Params.formatInput, m_Params.formatOutput, m_Params.srcColorSpace, m_Params.dstColorSpace, GetFeatureType());

        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamHdr m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterHdr)
};

struct FeatureParamLumakey : public FeatureParam
{
    PVPHAL_LUMAKEY_PARAMS lumaKeyParams = nullptr;
};

class SwFilterLumakey : public SwFilter
{
public:
    SwFilterLumakey(VpInterface &vpInterface);
    virtual ~SwFilterLumakey();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamLumakey &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS       AddFeatureGraphRTLog()
    {
        MT_LOG3(MT_VP_FEATURE_GRAPH_SWFILTERLUMAKEY, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_LUMAHIGH, m_Params.lumaKeyParams ? m_Params.lumaKeyParams->LumaHigh : -1, MT_VP_FEATURE_GRAPH_FILTER_LUMALOW,
                m_Params.lumaKeyParams ? m_Params.lumaKeyParams->LumaLow : -1, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterLumakey: LumaHigh %d, LumaLow %d, FeatureType %d", m_Params.lumaKeyParams ? m_Params.lumaKeyParams->LumaHigh : -1, m_Params.lumaKeyParams ? m_Params.lumaKeyParams->LumaLow : -1,
                                GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamLumakey m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterLumakey)
};

struct FeatureParamBlending : public FeatureParam
{
    PVPHAL_BLENDING_PARAMS  blendingParams = nullptr;
};

class SwFilterBlending : public SwFilter
{
public:
    SwFilterBlending(VpInterface &vpInterface);
    virtual ~SwFilterBlending();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamBlending &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS       AddFeatureGraphRTLog()
    {
        MT_LOG3(MT_VP_FEATURE_GRAPH_SWFILTERBLENDING, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_BLENDTYPE, m_Params.blendingParams ? m_Params.blendingParams->BlendType : -1, MT_VP_FEATURE_GRAPH_FILTER_FALPHA,
                m_Params.blendingParams ? (int64_t) m_Params.blendingParams->fAlpha : -1, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterBlending: BlendType %d, fAlpha %f, FeatureType %d", m_Params.blendingParams ? m_Params.blendingParams->BlendType : -1,
                                m_Params.blendingParams ? m_Params.blendingParams->fAlpha : -1, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamBlending m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterBlending)
};

struct FeatureParamColorFill : public FeatureParam
{
    PVPHAL_COLORFILL_PARAMS colorFillParams = nullptr;     //!< ColorFill - BG only
    bool operator == (const struct FeatureParamColorFill &b)
    {
        return (nullptr == colorFillParams && nullptr == b.colorFillParams ||
            nullptr != colorFillParams && nullptr != b.colorFillParams &&
            0 == memcmp(colorFillParams, b.colorFillParams, sizeof(*b.colorFillParams)));
    }
};

class SwFilterColorFill : public SwFilter
{
public:
    SwFilterColorFill(VpInterface &vpInterface);
    virtual ~SwFilterColorFill();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamColorFill &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual VP_EngineEntry   GetCombinedFilterEngineCaps(SwFilterSubPipe *singleInputPipeSelected);
    virtual MOS_STATUS       AddFeatureGraphRTLog()
    {
        MT_LOG3(MT_VP_FEATURE_GRAPH_SWFILTERCOLORFILL, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_INPUTCOLORSPACE, m_Params.colorFillParams ? m_Params.colorFillParams->CSpace : -1,
                MT_VP_FEATURE_GRAPH_FILTER_DISABLECFINSFC, m_Params.colorFillParams ? m_Params.colorFillParams->bDisableColorfillinSFC : -1, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterColorFill: CSpace %d, bDisableColorfillinSFC %d, FeatureType %d", m_Params.colorFillParams ? m_Params.colorFillParams->CSpace : -1,
                                m_Params.colorFillParams ? m_Params.colorFillParams->bDisableColorfillinSFC : -1, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamColorFill m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterColorFill)
};

struct FeatureParamAlpha : public FeatureParam
{
    PVPHAL_ALPHA_PARAMS     compAlpha         = nullptr;      //!< Alpha for composited surface
    bool                    calculatingAlpha  = false;        //!< Alpha calculation parameters
    bool operator == (const struct FeatureParamAlpha &b)
    {
        return calculatingAlpha == b.calculatingAlpha &&
            (nullptr == compAlpha   && nullptr == b.compAlpha ||
            nullptr != compAlpha    && nullptr != b.compAlpha &&
            0 == memcmp(compAlpha, b.compAlpha, sizeof(*b.compAlpha)));
    }
};

class SwFilterAlpha : public SwFilter
{
public:
    SwFilterAlpha(VpInterface &vpInterface);
    virtual ~SwFilterAlpha();
    virtual MOS_STATUS       Clean();
    virtual MOS_STATUS       Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual FeatureParamAlpha &GetSwFilterParams();
    virtual SwFilter *       Clone();
    virtual bool             operator==(SwFilter &swFilter);
    virtual MOS_STATUS       Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS       AddFeatureGraphRTLog()
    {
        MT_LOG4(MT_VP_FEATURE_GRAPH_SWFILTERALPHA, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_CALCULATINGALPHA, m_Params.calculatingAlpha, MT_VP_FEATURE_GRAPH_FILTER_ALPHAMODE,
                m_Params.compAlpha ? m_Params.compAlpha->AlphaMode : -1, MT_VP_FEATURE_GRAPH_FILTER_FALPHA, m_Params.compAlpha ? (int64_t) m_Params.compAlpha->fAlpha*1000 : -1,
                MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterAlpha: calculatingAlpha %d, AlphaMode %d, fAlpha %f, FeatureType %d", m_Params.calculatingAlpha, m_Params.compAlpha ? m_Params.compAlpha->AlphaMode : -1,
                                m_Params.compAlpha ? m_Params.compAlpha->fAlpha : -1, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamAlpha m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterAlpha)
};

struct FeatureParamCgc : public FeatureParam
{
    VPHAL_GAMUT_MODE    GCompMode           = GAMUT_MODE_NONE;
    VPHAL_CSPACE        colorSpace          = CSpace_None;
    VPHAL_CSPACE        dstColorSpace       = CSpace_None;
    bool                bBt2020ToRGB        = false;
    bool                bExtendedSrcGamut   = false;
    bool                bExtendedDstGamut   = false;
    uint32_t            dwAttenuation       = 0;                //!< U2.10 [0, 1024] 0 = No down scaling, 1024 = Full down scaling
    float               displayRGBW_x[4]    = {};
    float               displayRGBW_y[4]    = {};
};

class SwFilterCgc : public SwFilter
{
public:
    SwFilterCgc(VpInterface& vpInterface);
    virtual ~SwFilterCgc();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamCgc& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);
    virtual bool IsBt2020ToRGB(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual bool IsBt2020ToRGBEnabled()
    {
        return m_Params.bBt2020ToRGB;
    }
    virtual MOS_STATUS AddFeatureGraphRTLog()
    {
        MT_LOG5(MT_VP_FEATURE_GRAPH_SWFILTERCGC, MT_NORMAL, MT_VP_FEATURE_GRAPH_FILTER_GCOMPMODE, m_Params.GCompMode, MT_VP_FEATURE_GRAPH_FILTER_INPUTCOLORSPACE, m_Params.colorSpace,
                MT_VP_FEATURE_GRAPH_FILTER_OUTPUTCOLORSPACE, m_Params.dstColorSpace, MT_VP_FEATURE_GRAPH_FILTER_BT2020TORGB, m_Params.bBt2020ToRGB, MT_VP_FEATURE_GRAPH_FILTER_FEATURETYPE, GetFeatureType());
        VP_PUBLIC_NORMALMESSAGE("Feature Graph: SwFilterCgc: GCompMode %d, srccolorSpace %d, dstColorSpace %d, bBt2020ToRGB %d, FeatureType %d", m_Params.GCompMode, m_Params.colorSpace,
                                m_Params.dstColorSpace, m_Params.bBt2020ToRGB, GetFeatureType());
        return MOS_STATUS_SUCCESS;
    }

protected:
    FeatureParamCgc m_Params = {};

MEDIA_CLASS_DEFINE_END(vp__SwFilterCgc)
};

class SwFilterSet
{
public:
    SwFilterSet();
    virtual ~SwFilterSet();

    MOS_STATUS AddSwFilter(SwFilter *swFilter);
    MOS_STATUS RemoveSwFilter(SwFilter *swFilter);
    MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    MOS_STATUS AddFeatureGraphRTLog();
    MOS_STATUS Clean();
    SwFilter *GetSwFilter(FeatureType type);
    bool IsEmpty()
    {
        return m_swFilters.empty();
    }

    std::vector<class SwFilterSet *> *GetLocation();
    void SetLocation(std::vector<class SwFilterSet *> *location);
    RenderTargetType                  GetRenderTargetType();

private:
    std::map<FeatureType, SwFilter *> m_swFilters;
    // nullptr if it is unordered filters, otherwise, it's the pointer to m_OrderedFilters it belongs to.
    std::vector<class SwFilterSet *> *m_location = nullptr;

MEDIA_CLASS_DEFINE_END(vp__SwFilterSet)
};

}
#endif // !__SW_FILTER_H__
