/*
* Copyright (c) 2019-2021, Intel Corporation
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
#include <vector>
#include "media_sfc_interface.h"

namespace vp
{
class VpInterface;
class SwFilterSubPipe;
enum FeatureType
{
    FeatureTypeInvalid          = 0,
    FeatureTypeCsc              = 0x100,
    FeatureTypeCscOnSfc,
    FeatureTypeCscOnVebox,
    FeatureTypeCscOnRender,
    FeatureTypeRotMir           = 0x200,
    FeatureTypeRotMirOnSfc,
    FeatureTypeRotMirOnRender,
    FeatureTypeScaling          = 0x300,
    FeatureTypeScalingOnSfc,
    FeatureTypeScalingOnRender,
    FeatureTypeDn               = 0x400,
    FeatureTypeDnOnVebox,
    FeatureTypeDi               = 0x500,
    FeatureTypeDiOnVebox,
    FeatureTypeSte              = 0x600,
    FeatureTypeSteOnVebox,
    FeatureTypeAce              = 0x700,
    FeatureTypeAceOnVebox,
    FeatureTypeVeboxUpdate      = 0x800,
    FeatureTypeVeboxUpdateOnRender,
    FeatureTypeTcc              = 0x900,
    FeatureTypeTccOnVebox,
    FeatureTypeProcamp          = 0xA00,
    FeatureTypeProcampOnVebox,
    FeatureTypeProcampOnRender,
    FeatureTypeCgc              = 0xB00,
    FeatureTypeCgcOnVebox,
    FeatureTypeHdr              = 0xC00,
    FeatureTypeHdrOnVebox,
    FeatureTypeFD               = 0xD00,
    FeatureTypeFLD              = 0xE00,
    FeatureTypeFB               = 0xF00,
    FeatureTypeSecureCopy      = 0x1000,
    FeatureTypeSecureCopyOnRender,
    // ...
    NumOfFeatureType
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
    SurfaceTypeAutoDNNoiseLevel, // with kernel path needed
    SurfaceTypeAutoDNSpatialConfig,
    SurfaceTypeACEHistory,
    SurfaceTypeFMDHistory,
    SurfaceTypeLaceAceRGBHistogram,
    SurfaceTypeLaceLut,
    SurfaceTypeStatistics,
    SurfaceTypeSkinScore,
    SurfaceType3dLut,
    SurfaceType1dLutHDR,
    SurfaceTypeAlphaOrVignette,
    SurfaceTypeVeboxStateHeap_Drv,
    SurfaceTypeVeboxStateHeap_Knr,
    NumberOfSurfaceType
};

typedef std::map<SurfaceType, VP_SURFACE*> VP_SURFACE_GROUP;

struct VP_SURFACE_SETTING
{
    VP_SURFACE_GROUP    surfGroup;
    bool                isPastHistogramValid;
    uint32_t            imageWidthOfPastHistogram;
    uint32_t            imageHeightOfPastHistogram;

    void Clean()
    {
        surfGroup.clear();
        isPastHistogramValid        = false;
        imageWidthOfPastHistogram   = 0;
        imageHeightOfPastHistogram  = 0;
    }
};

#define FEATURE_TYPE_MASK   0xffffff00

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
    FeatureType type;
    MOS_FORMAT  formatInput;
    MOS_FORMAT  formatOutput;
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
        return MOS_STATUS_SUCCESS;
    }
    virtual FeatureType GetFeatureType()
    {
        return m_type;
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

    // The child class need to implement SetResourceAssignmentHint only when any feature
    // parameters will affect the resource assignment.
    virtual MOS_STATUS SetResourceAssignmentHint(RESOURCE_ASSIGNMENT_HINT &hint)
    {
        return MOS_STATUS_SUCCESS;
    }

protected:
    VpInterface &m_vpInterface;
    FeatureType m_type = FeatureTypeInvalid;
    // SwFilterSet current swFilter belongs to.
    SwFilterSet *m_location = nullptr;
    VP_EngineEntry  m_EngineCaps = {};
    bool m_noNeedUpdate = false;
};

struct FeatureParamCsc : public FeatureParam
{
    struct CSC_PARAMS
    {
        VPHAL_CSPACE    colorSpace;
        uint32_t        chromaSiting;
    };
    CSC_PARAMS          input;
    CSC_PARAMS          output;
    PVPHAL_IEF_PARAMS   pIEFParams;
    PVPHAL_ALPHA_PARAMS pAlphaParams;
    FeatureParamCsc     *next;                //!< pointe to new/next generated CSC params
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
    virtual FeatureParamCsc &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);
    virtual MOS_STATUS SetFeatureType(FeatureType type);

private:
    FeatureParamCsc m_Params = {};
};

struct FeatureParamScaling : public FeatureParam
{
    struct SCALING_PARAMS
    {
        uint32_t                dwWidth;
        uint32_t                dwHeight;
        RECT                    rcSrc;
        RECT                    rcDst;                          //!< Input dst rect without rotate being applied.
        RECT                    rcMaxSrc;
        VPHAL_SAMPLE_TYPE       sampleType;
    };

    // Parameters maintained by scaling feature parameters
    SCALING_PARAMS              input;
    SCALING_PARAMS              output;
    VPHAL_SCALING_MODE          scalingMode;
    VPHAL_SCALING_PREFERENCE    scalingPreference;              //!< DDI indicate Scaling preference
    bool                        bDirectionalScalar = false;     //!< Vebox Directional Scalar
    PVPHAL_COLORFILL_PARAMS     pColorFillParams;               //!< ColorFill - BG only
    PVPHAL_ALPHA_PARAMS         pCompAlpha;                     //!< Alpha for composited surfaces
    VPHAL_ISCALING_TYPE         interlacedScalingType;

    // Parameters maintained by other feature parameters.
    struct {
        VPHAL_CSPACE            colorSpaceOutput;
    } csc;

    struct {
        bool                    rotationNeeded;                 //!< Whether rotate SwFilter exists on SwFilterPipe.
    } rotation;

    FeatureParamScaling        *next;                           //!< pointe to new/next generated scaling params
};

class SwFilterScaling : public SwFilter
{
public:
    SwFilterScaling(VpInterface &vpInterface);
    virtual ~SwFilterScaling();
    virtual MOS_STATUS Clean();
    virtual MOS_STATUS Configure(VP_PIPELINE_PARAMS &params, bool isInputSurf, int surfIndex);
    virtual MOS_STATUS Configure(VEBOX_SFC_PARAMS &params);
    virtual FeatureParamScaling &GetSwFilterParams();
    virtual SwFilter *Clone();
    virtual bool operator == (SwFilter& swFilter);
    virtual MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf, SwFilterSubPipe &pipe);

private:
    FeatureParamScaling m_Params = {};
};

struct FeatureParamRotMir : public FeatureParam
{
    // Parameters maintained by rotation feature parameters
    VPHAL_ROTATION rotation;

    // Parameters maintained by other feature parameters.
    struct {
        MOS_TILE_TYPE  tileOutput;
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
};

struct FeatureParamDenoise : public FeatureParam
{
    VPHAL_SAMPLE_TYPE    sampleTypeInput;
    VPHAL_DENOISE_PARAMS denoiseParams;
    uint32_t             widthAlignUnitInput;
    uint32_t             heightAlignUnitInput;
    uint32_t             heightInput;
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
};

struct FeatureParamDeinterlace : public FeatureParam
{
    VPHAL_SAMPLE_TYPE       sampleTypeInput;
    bool                    bHDContent;
    VPHAL_DI_MODE           DIMode;             //!< DeInterlacing mode
    bool                    bEnableFMD;         //!< FMD
    bool                    b60fpsDi;           //!< Used in frame Recon - if 30fps (one call per sample pair)
    bool                    bSCDEnable;         //!< Scene change detection
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
        hint.bDi = 1;
        hint.b60fpsDi = m_Params.b60fpsDi;
        return MOS_STATUS_SUCCESS;
    }

private:
    FeatureParamDeinterlace m_Params = {};
};

struct FeatureParamSte : public FeatureParam
{
    bool                bEnableSTE;
    uint32_t            dwSTEFactor;
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
};

struct FeatureParamTcc : public FeatureParam
{
    bool                bEnableTCC;
    uint8_t             Red;
    uint8_t             Green;
    uint8_t             Blue;
    uint8_t             Cyan;
    uint8_t             Magenta;
    uint8_t             Yellow;
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
};

struct FeatureParamProcamp : public FeatureParam
{
    bool                bEnableProcamp;
    float               fBrightness;
    float               fContrast;
    float               fHue;
    float               fSaturation;
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
};

struct FeatureParamHdr : public FeatureParam
{
    uint32_t        uiMaxDisplayLum;       //!< Maximum Display Luminance
    uint32_t        uiMaxContentLevelLum;  //!< Maximum Content Level Luminance
    VPHAL_HDR_MODE  hdrMode;
    VPHAL_CSPACE    srcColorSpace;
    VPHAL_CSPACE    dstColorSpace;
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

private:
    FeatureParamHdr m_Params = {};
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

private:
    std::map<FeatureType, SwFilter *> m_swFilters;
    // nullptr if it is unordered filters, otherwise, it's the pointer to m_OrderedFilters it belongs to.
    std::vector<class SwFilterSet *> *m_location = nullptr;
};

}
#endif // !__SW_FILTER_H__
