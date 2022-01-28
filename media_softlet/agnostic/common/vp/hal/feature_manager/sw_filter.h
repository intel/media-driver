/*
* Copyright (c) 2019-2022, Intel Corporation
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

enum FeatureType
{
    FeatureTypeInvalid          = 0,
    FeatureTypeCsc              = 0x100,
    FeatureTypeCscOnSfc         = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeCscOnVebox       = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeCscOnRender      = FeatureTypeCsc | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeRotMir           = 0x200,
    FeatureTypeRotMirOnSfc      = FeatureTypeRotMir | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeRotMirOnRender   = FeatureTypeRotMir | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeScaling          = 0x300,
    FeatureTypeScalingOnSfc     = FeatureTypeScaling | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeScalingOnRender  = FeatureTypeScaling | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeDn               = 0x400,
    FeatureTypeDnOnVebox        = FeatureTypeDn | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeDi               = 0x500,
    FeatureTypeDiOnVebox        = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeDiOnRender       = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeDiFmdOnRender    = FeatureTypeDi | FEATURE_TYPE_ENGINE_BITS_RENDER | FEATURE_TYPE_ENGINE_BITS_SUB_STEP,
    FeatureTypeSte              = 0x600,
    FeatureTypeSteOnVebox       = FeatureTypeSte | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeAce              = 0x700,
    FeatureTypeAceOnVebox       = FeatureTypeAce | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeVeboxUpdate      = 0x800,
    FeatureTypeVeboxUpdateOnRender = FeatureTypeVeboxUpdate | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeTcc              = 0x900,
    FeatureTypeTccOnVebox       = FeatureTypeTcc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeProcamp          = 0xA00,
    FeatureTypeProcampOnVebox   = FeatureTypeProcamp | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeProcampOnRender  = FeatureTypeProcamp | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeCgc              = 0xB00,
    FeatureTypeCgcOnVebox       = FeatureTypeCgc | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeHdr              = 0xC00,
    FeatureTypeHdrOnVebox       = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeHdr3DLutCalOnRender = FeatureTypeHdr | FEATURE_TYPE_ENGINE_BITS_RENDER | FEATURE_TYPE_ENGINE_BITS_SUB_STEP,
    FeatureTypeFD               = 0xD00,
    FeatureTypeFLD              = 0xE00,
    FeatureTypeFB               = 0xF00,
    FeatureTypeSecureCopy       = 0x1000,
    FeatureTypeSecureCopyOnRender = FeatureTypeSecureCopy | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeSR               = 0x1100,
    FeatureTypeSROnRender       = FeatureTypeSR | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeLace             = 0x1200,
    FeatureTypeLaceOnVebox      = FeatureTypeLace | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeLaceOnRender     = FeatureTypeLace | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeGamutExt         = 0x1300,
    FeatureTypeDV               = 0x1400,
    FeatureTypeDVOnVebox        = FeatureTypeDV | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeFc               = 0x1500,
    FeatureTypeFcOnRender       = FeatureTypeFc | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeLumakey          = 0x1600,
    FeatureTypeLumakeyOnRender  = FeatureTypeLumakey | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeBlending         = 0x1700,
    FeatureTypeBlendingOnRender = FeatureTypeBlending | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeColorFill        = 0x1800,
    FeatureTypeColorFillOnSfc   = FeatureTypeColorFill | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeColorFillOnRender = FeatureTypeColorFill | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeAlpha            = 0x1900,
    FeatureTypeAlphaOnSfc       = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_SFC,
    FeatureTypeAlphaOnVebox     = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeAlphaOnRender    = FeatureTypeAlpha | FEATURE_TYPE_ENGINE_BITS_RENDER,
    FeatureTypeCappipe          = 0x2000,
    FeatureTypeCappipeOnVebox   = FeatureTypeCappipe | FEATURE_TYPE_ENGINE_BITS_VEBOX,
    FeatureTypeCappipeOnRender  = FeatureTypeCappipe | FEATURE_TYPE_ENGINE_BITS_RENDER,
    // ...
    NumOfFeatureType
};

enum RenderTargetType
{
    RenderTargetTypeInvalid = 0,
    RenderTargetTypeSurface,
    RenderTargetTypeParameter
};

enum SurfaceType
{
    SurfaceTypeInvalid = 0,
    SurfaceTypeVeboxInput,
    SurfaceTypeVeboxPreviousInput,
    SurfaceTypeDNOutput,
    SurfaceTypeVeboxCurrentOutput,
    SurfaceTypeVeboxPreviousOutput,
    SurfaceTypeScalar,
    SurfaceTypeSTMMIn,
    SurfaceTypeSTMMOut,
    SurfaceTypeACEHistory,
    SurfaceTypeFMDHistory,
    SurfaceTypeLaceAceRGBHistogram,
    SurfaceTypeLaceLut,
    SurfaceTypeStatistics,
    SurfaceTypeSkinScore,
    SurfaceType3DLut,
    SurfaceType1k1dLut,
    SurfaceType1dLutHDR,
    SurfaceTypeAlphaOrVignette,
    SurfaceTypeVeboxStateHeap_Drv,
    SurfaceTypeVeboxStateHeap_Knr,
    SurfaceTypeAutoDNNoiseLevel, // with kernel path needed
    SurfaceTypeAutoDNSpatialConfig,
    SurfaceTypeRenderInput,
    SurfaceTypeRenderOutput,
    SurfaceTypeRenderSRInput, //Super Resolution related Surface and Buffer index Reserved
    SurfaceTypeRenderSRBuffer = SurfaceTypeRenderSRInput + 0x100,
    SurfaceTypeRenderSRMax = SurfaceTypeRenderSRBuffer + 0x100,
    SurfaceTypeAggregatedHistogram,
    SurfaceTypeFrameHistogram,
    SurfaceTypeStdStatistics,
    SurfaceTypePwlfIn,
    SurfaceTypePwlfOut,
    SurfaceTypeWeitCoef,
    SurfaceTypGlobalToneMappingCurveLUT,
    // FC
    SurfaceTypeFcInputLayer0,
    SurfaceTypeFcInputLayer1,
    SurfaceTypeFcInputLayer2,
    SurfaceTypeFcInputLayer3,
    SurfaceTypeFcInputLayer4,
    SurfaceTypeFcInputLayer5,
    SurfaceTypeFcInputLayer6,
    SurfaceTypeFcInputLayer7,
    SurfaceTypeFcInputLayerMax = SurfaceTypeFcInputLayer7,
    SurfaceTypeFcInputLayer0Field1Dual,
    SurfaceTypeFcInputLayer1Field1Dual,
    SurfaceTypeFcInputLayer2Field1Dual,
    SurfaceTypeFcInputLayer3Field1Dual,
    SurfaceTypeFcInputLayer4Field1Dual,
    SurfaceTypeFcInputLayer5Field1Dual,
    SurfaceTypeFcInputLayer6Field1Dual,
    SurfaceTypeFcInputLayer7Field1Dual,
    SurfaceTypeFcInputLayerMaxField1Dual = SurfaceTypeFcInputLayer7Field1Dual,
    SurfaceTypeFcTarget0,
    SurfaceTypeFcTarget1,
    SurfaceTypeFcCscCoeff,
    // LGCA related Surfaces
    SurfaceTypeSamplerSurfaceR,
    SurfaceTypeSamplerSurfaceG,
    SurfaceTypeSamplerSurfaceB,
    SurfaceTypeOutputSurfaceR,
    SurfaceTypeOutputSurfaceG,
    SurfaceTypeOutputSurfaceB,
    SurfaceTypeSamplerParamsMinMax,
    // 3DLut Kernel
    SurfaceType3DLut2D,
    SurfaceType3DLutCoef,
    NumberOfSurfaceType
};

using  VP_SURFACE_GROUP = std::map<SurfaceType, VP_SURFACE*>;

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

    void SetExePipeFlag(bool isInExePipe)
    {
        m_isInExePipe = isInExePipe;
    }

protected:
    VpInterface &m_vpInterface;
    FeatureType m_type = FeatureTypeInvalid;
    // SwFilterSet current swFilter belongs to.
    SwFilterSet *m_location = nullptr;
    VP_EngineEntry  m_EngineCaps = {};
    bool m_noNeedUpdate = false;
    RenderTargetType m_renderTargetType = RenderTargetTypeSurface;
    bool m_isInExePipe = false;

MEDIA_CLASS_DEFINE_END(SwFilter)
};

struct FeatureParamCsc : public FeatureParam
{
    struct CSC_PARAMS
    {
        VPHAL_CSPACE    colorSpace      = CSpace_None;
        uint32_t        chromaSiting    = 0;
    };
    CSC_PARAMS          input           = {};
    CSC_PARAMS          output          = {};
    PVPHAL_IEF_PARAMS   pIEFParams      = nullptr;
    PVPHAL_ALPHA_PARAMS pAlphaParams    = nullptr;
    FeatureParamCsc     *next           = nullptr;                //!< pointe to new/next generated CSC params
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

private:
    FeatureParamCsc m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterCsc)
};

struct FeatureParamScaling : public FeatureParam
{
    struct SCALING_PARAMS
    {
        uint32_t                dwWidth  = 0;
        uint32_t                dwHeight = 0;
        RECT                    rcSrc    = {0, 0, 0, 0};
        RECT                    rcDst    = {0, 0, 0, 0};  //!< Input dst rect without rotate being applied.
        RECT                    rcMaxSrc = {0, 0, 0, 0};
        VPHAL_SAMPLE_TYPE       sampleType = SAMPLE_PROGRESSIVE;
    };

    // Parameters maintained by scaling feature parameters
    SCALING_PARAMS              input       = {};
    SCALING_PARAMS              output      = {};
    bool                        isPrimary   = false;
    VPHAL_SCALING_MODE          scalingMode = VPHAL_SCALING_NEAREST;
    VPHAL_SCALING_PREFERENCE    scalingPreference  = VPHAL_SCALING_PREFER_SFC;  //!< DDI indicate Scaling preference
    bool                        bDirectionalScalar = false;     //!< Vebox Directional Scalar
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

private:
    FeatureParamScaling m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterScaling)
};

struct FeatureParamRotMir : public FeatureParam
{
    // Parameters maintained by rotation feature parameters
    VPHAL_ROTATION rotation = VPHAL_ROTATION_IDENTITY;

    // Parameters maintained by other feature parameters.
    struct {
        MOS_TILE_TYPE tileOutput = MOS_TILE_X;
    } surfInfo;
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

private:
    FeatureParamRotMir m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterRotMir)
};

struct FeatureParamDenoise : public FeatureParam
{
    VPHAL_SAMPLE_TYPE    sampleTypeInput      = SAMPLE_PROGRESSIVE;
    VPHAL_DENOISE_PARAMS denoiseParams        = {};
    uint32_t             widthAlignUnitInput  = 0;
    uint32_t             heightAlignUnitInput = 0;
    uint32_t             heightInput          = 0;
};

class SwFilterDenoise : public SwFilter
{
public:
    SwFilterDenoise(VpInterface& vpInterface);
    virtual ~SwFilterDenoise();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS& params, bool isInputSurf, int surfIndex);
    virtual FeatureParamDenoise& GetSwFilterParams();
    virtual SwFilter* Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE* inputSurf, VP_SURFACE* outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamDenoise m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterDenoise)
};

struct FeatureParamDeinterlace : public FeatureParam
{
    VPHAL_SAMPLE_TYPE       sampleTypeInput      = SAMPLE_PROGRESSIVE;
    bool                    bHDContent           = false;
    PVPHAL_DI_PARAMS        diParams             = nullptr;
    bool                    bFmdExtraVariance    = false;     //!< Check if extra FMD variances need to be calculated
    bool                    bFmdKernelEnable     = false;     //!< FMD kernel path enabled
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

private:
    FeatureParamDeinterlace m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterDeinterlace)
};

struct FeatureParamSte : public FeatureParam
{
    bool       bEnableSTE  = false;
    uint32_t   dwSTEFactor = 0;
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

private:
    FeatureParamSte m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterSte)
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

private:
    FeatureParamTcc m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterTcc)
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

private:
    FeatureParamProcamp m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterProcamp)
};

enum HDR_STAGE
{
    HDR_STAGE_DEFAULT = 0,
    HDR_STAGE_3DLUT_KERNEL,
    HDR_STAGE_VEBOX_3DLUT_UPDATE,
    HDR_STAGE_VEBOX_3DLUT_NO_UPDATE,
};

struct FeatureParamHdr : public FeatureParam
{
    uint32_t        uiMaxDisplayLum      = 0;  //!< Maximum Display Luminance
    uint32_t        uiMaxContentLevelLum = 0;  //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE  hdrMode              = VPHAL_HDR_MODE_NONE;
    VPHAL_CSPACE    srcColorSpace        = CSpace_None;
    VPHAL_CSPACE    dstColorSpace        = CSpace_None;

    HDR_STAGE       stage;
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

private:
    FeatureParamHdr m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterHdr)
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

private:
    FeatureParamLumakey m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterLumakey)
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

private:
    FeatureParamBlending m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterBlending)
};

struct FeatureParamColorFill : public FeatureParam
{
    PVPHAL_COLORFILL_PARAMS colorFillParams = nullptr;     //!< ColorFill - BG only
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

private:
    FeatureParamColorFill m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterColorFill)
};

struct FeatureParamAlpha : public FeatureParam
{
    PVPHAL_ALPHA_PARAMS     compAlpha         = nullptr;      //!< Alpha for composited surface
    bool                    calculatingAlpha  = false;        //!< Alpha calculation parameters
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

private:
    FeatureParamAlpha m_Params = {};

MEDIA_CLASS_DEFINE_END(SwFilterAlpha)
};

class SwFilterSet
{
public:
    SwFilterSet();
    virtual ~SwFilterSet();

    MOS_STATUS AddSwFilter(SwFilter *swFilter);
    MOS_STATUS RemoveSwFilter(SwFilter *swFilter);
    MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
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

MEDIA_CLASS_DEFINE_END(SwFilterSet)
};

}
#endif // !__SW_FILTER_H__
