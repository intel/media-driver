/*
* Copyright (c) 2018-2022, Intel Corporation
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
//! \file     vp_resource_manager.h
//! \brief    The header file of the base class of vp resource manager
//! \details  all the vp resources will be traced here for usages using intermeida 
//!           surfaces.
//!
#ifndef _VP_RESOURCE_MANAGER_H__
#define _VP_RESOURCE_MANAGER_H__

#include <map>
#include "vp_allocator.h"
#include "vp_pipeline_common.h"
#include "vp_utils.h"
#include "vp_hdr_resource_manager.h"
#include "media_copy_wrapper.h"

#define VP_MAX_NUM_VEBOX_SURFACES     4                                       //!< Vebox output surface creation, also can be reuse for DI usage:
                                                                              //!< for DI: 2 for ADI plus additional 2 for parallel execution
#define VP_NUM_DN_SURFACES           2                                       //!< Number of DN output surfaces
#define VP_NUM_STMM_SURFACES         2                                       //!< Number of STMM statistics surfaces
#define VP_DNDI_BUFFERS_MAX          4                                       //!< Max DNDI buffers
#define VP_NUM_KERNEL_VEBOX          8                                       //!< Max kernels called at Adv stage

#define VP_VEBOX_PER_BLOCK_STATISTICS_SIZE   16
#define VP_VEBOX_FMD_HISTORY_SIZE            (144 * sizeof(uint32_t))

#define VP_NUM_ACE_STATISTICS_HISTOGRAM      256
#define VP_NUM_STD_STATISTICS                2

#ifndef VEBOX_AUTO_DENOISE_SUPPORTED
#define VEBOX_AUTO_DENOISE_SUPPORTED    1
#endif

//!
//! \brief Number of LACE's PWLF surfaces
//!
#define VP_NUM_LACE_PWLF_SURFACES                    2

#define VP_MAX_HDR_INPUT_LAYER                       8

#define IS_VP_VEBOX_DN_ONLY(_a) (_a.bDN &&          \
                               !(_a.bDI) &&   \
                               !(_a.bQueryVariance) && \
                               !(_a.bIECP) && \
                               !(_a.b3DlutOutput))

#define VP_NUM_FC_INTERMEDIA_SURFACES   2

namespace vp {
    struct VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION
    {
        // DWORD 0
        union
        {
            // RangeThrStart0
            struct
            {
                uint32_t       RangeThrStart0;
            };

            uint32_t       Value;
        } DW00;

        // DWORD 1
        union
        {
            // RangeThrStart1
            struct
            {
                uint32_t       RangeThrStart1;
            };

            uint32_t       Value;
        } DW01;

        // DWORD 2
        union
        {
            // RangeThrStart2
            struct
            {
                uint32_t       RangeThrStart2;
            };

            uint32_t   Value;
        } DW02;

        // DWORD 3
        union
        {
            // RangeThrStart3
            struct
            {
                uint32_t       RangeThrStart3;
            };

            uint32_t   Value;
        } DW03;

        // DWORD 4
        union
        {
            // RangeThrStart4
            struct
            {
                uint32_t       RangeThrStart4;
            };

            uint32_t   Value;
        } DW04;

        // DWORD 5
        union
        {
            // RangeThrStart5
            struct
            {
                uint32_t       RangeThrStart5;
            };

            uint32_t   Value;
        } DW05;

        // DWORD 6
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW06;

        // DWORD 7
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW07;

        // DWORD 8
        union
        {
            // RangeWgt0
            struct
            {
                uint32_t       RangeWgt0;
            };

            uint32_t   Value;
        } DW08;

        // DWORD 9
        union
        {
            // RangeWgt1
            struct
            {
                uint32_t       RangeWgt1;
            };

            uint32_t   Value;
        } DW09;

        // DWORD 10
        union
        {
            // RangeWgt2
            struct
            {
                uint32_t       RangeWgt2;
            };

            uint32_t   Value;
        } DW10;

        // DWORD 11
        union
        {
            // RangeWgt3
            struct
            {
                uint32_t       RangeWgt3;
            };

            uint32_t   Value;
        } DW11;

        // DWORD 12
        union
        {
            // RangeWgt4
            struct
            {
                uint32_t       RangeWgt4;
            };

            uint32_t   Value;
        } DW12;

        // DWORD 13
        union
        {
            // RangeWgt5
            struct
            {
                uint32_t       RangeWgt5;
            };

            uint32_t   Value;
        } DW13;

        // DWORD 14
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW14;

        // DWORD 15
        union
        {
            // Reserved
            struct
            {
                uint32_t       Reserved;
            };

            uint32_t   Value;
        } DW15;

        // DWORD 16 - 41: DistWgt[5][5]
        uint32_t DistWgt[5][5];

        // Padding for 32-byte alignment, VEBOX_SPATIAL_ATTRIBUTES_CONFIGURATION_G9 is 7 uint32_ts
        uint32_t dwPad[7];
    };

enum VEBOX_SURFACE_ID
{
    VEBOX_SURFACE_NULL = 0,
    VEBOX_SURFACE_INPUT,
    VEBOX_SURFACE_OUTPUT,
    VEBOX_SURFACE_PAST_REF,
    VEBOX_SURFACE_FUTURE_REF,
    VEBOX_SURFACE_FRAME0,
    VEBOX_SURFACE_FRAME1,
    VEBOX_SURFACE_FRAME2,
    VEBOX_SURFACE_FRAME3,
};

struct VEBOX_SURFACES
{
    VEBOX_SURFACE_ID currentInputSurface;
    VEBOX_SURFACE_ID pastInputSurface;
    VEBOX_SURFACE_ID currentOutputSurface;
    VEBOX_SURFACE_ID pastOutputSurface;

    VEBOX_SURFACES(VEBOX_SURFACE_ID _currentInputSurface, VEBOX_SURFACE_ID _pastInputSurface, VEBOX_SURFACE_ID _currentOutputSurface, VEBOX_SURFACE_ID _pastOutputSurface)
        : currentInputSurface(_currentInputSurface), pastInputSurface(_pastInputSurface), currentOutputSurface(_currentOutputSurface), pastOutputSurface(_pastOutputSurface)
    {}
};

inline uint32_t BoolToInt(bool b)
{
    return ((b) ? 1 : 0);
}

union VEBOX_SURFACES_CONFIG
{
    struct
    {
        uint32_t b64DI              : 1;
        uint32_t sfcEnable          : 1;
        uint32_t sameSample         : 1;
        uint32_t outOfBound         : 1;
        uint32_t pastFrameAvailable    : 1;
        uint32_t futureFrameAvailable    : 1;
        uint32_t firstDiField       : 1;
        uint32_t reserved           : 25;
    };
    uint32_t value;
    VEBOX_SURFACES_CONFIG() : value(0)
    {}
    VEBOX_SURFACES_CONFIG(bool _b64DI, bool _sfcEnable, bool _sameSample, bool _outOfBound, bool _pastFrameAvailable, bool _futureFrameAvailable, bool _firstDiField) :
        b64DI(BoolToInt(_b64DI)), sfcEnable(BoolToInt(_sfcEnable)), sameSample(BoolToInt(_sameSample)), outOfBound(BoolToInt(_outOfBound)),
        pastFrameAvailable(BoolToInt(_pastFrameAvailable)), futureFrameAvailable(BoolToInt(_futureFrameAvailable)), firstDiField(BoolToInt(_firstDiField)), reserved(0)
    {}
};

typedef std::map<uint32_t, VEBOX_SURFACES> VEBOX_SURFACE_CONFIG_MAP;

struct VP_FRAME_IDS
{
    bool        valid;
    bool        diEnabled;
    int32_t     currentFrameId;
    bool        pastFrameAvailable;
    bool        futureFrameAvailable;
    int32_t     pastFrameId;
    int32_t     futureFrameId;
};

struct VP_SURFACE_PARAMS
{
    uint32_t              width               = 0;
    uint32_t              height              = 0;
    MOS_FORMAT            format              = Format_None;
    MOS_TILE_TYPE         tileType            = MOS_TILE_X;
    MOS_RESOURCE_MMC_MODE surfCompressionMode = MOS_MMC_DISABLED;
    bool                  surfCompressible    = false;
    VPHAL_CSPACE          colorSpace          = CSpace_None;
    RECT                  rcSrc               = {0, 0, 0, 0};  //!< Source rectangle
    RECT                  rcDst               = {0, 0, 0, 0};  //!< Destination rectangle
    RECT                  rcMaxSrc            = {0, 0, 0, 0};  //!< Max source rectangle
    VPHAL_SAMPLE_TYPE     sampleType          = SAMPLE_PROGRESSIVE;
    VPHAL_SURFACE_TYPE    surfType            = SURF_NONE;
};

class VpResourceManager
{
public:
    VpResourceManager(MOS_INTERFACE &osInterface, VpAllocator &allocator, VphalFeatureReport &reporting, vp::VpPlatformInterface &vpPlatformInterface, MediaCopyWrapper *mediaCopyWrapper, vp::VpUserFeatureControl *vpUserFeatureControl);
    virtual ~VpResourceManager();
    virtual MOS_STATUS OnNewFrameProcessStart(SwFilterPipe &pipe);
    virtual void OnNewFrameProcessEnd();
    MOS_STATUS PrepareFcIntermediateSurface(SwFilterPipe &featurePipe);
    MOS_STATUS GetResourceHint(std::vector<FeatureType> &featurePool, SwFilterPipe& executedFilters, RESOURCE_ASSIGNMENT_HINT &hint);
    MOS_STATUS AssignExecuteResource(std::vector<FeatureType> &featurePool, VP_EXECUTE_CAPS& caps, SwFilterPipe &executedFilters);
    MOS_STATUS AssignExecuteResource(VP_EXECUTE_CAPS& caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface,
        std::vector<VP_SURFACE *> &pastSurfaces, std::vector<VP_SURFACE *> &futureSurfaces,
        RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe& executedFilters);
    MOS_STATUS GetUpdatedExecuteResource(std::vector<FeatureType> &featurePool, VP_EXECUTE_CAPS &caps, SwFilterPipe &swfilterPipe, VP_SURFACE_SETTING &surfSetting);

    virtual MOS_STATUS FillLinearBufferWithEncZero(VP_SURFACE *surface, uint32_t width, uint32_t height);

    bool IsSameSamples()
    {
        return m_sameSamples;
    }

    bool IsOutputSurfaceNeeded(VP_EXECUTE_CAPS caps);

    bool IsRefValid()
    {
        return m_currentFrameIds.pastFrameAvailable || m_currentFrameIds.futureFrameAvailable;
    }

    bool IsPastHistogramValid()
    {
        return m_isPastHistogramValid;
    }

    void GetImageResolutionOfPastHistogram(uint32_t &width, uint32_t &height)
    {
        width = m_imageWidthOfPastHistogram;
        height = m_imageHeightOfPastHistogram;
    }

    virtual VP_SURFACE* GetVeboxLaceLut()
    {
        return NULL;
    }

    virtual VP_SURFACE* GetVeboxAggregatedHistogramSurface()
    {
       return NULL;
    }

    virtual VP_SURFACE* GetVeboxFrameHistogramSurface()
    {
       return NULL;
    }

    virtual VP_SURFACE* GetVeboxStdStatisticsSurface()
    {
       return NULL;
    }

    virtual VP_SURFACE* GetVeboxPwlfSurface()
    {
       return NULL;
    }

    virtual VP_SURFACE* GetVeboxWeitCoefSurface()
    {
       return NULL;
    }

    virtual VP_SURFACE* GetVeboxGlobalToneMappingCurveLUTSurface()
    {
       return NULL;
    }

protected:
    VP_SURFACE* GetVeboxOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *outputSurface);
    MOS_STATUS InitVeboxSpatialAttributesConfiguration();
    MOS_STATUS AllocateVeboxResource(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface);
    MOS_STATUS AssignSurface(VP_EXECUTE_CAPS caps, VEBOX_SURFACE_ID &surfaceId, SurfaceType surfaceType, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface, VP_SURFACE *pastRefSurface, VP_SURFACE *futureRefSurface, VP_SURFACE_GROUP &surfGroup);
    bool VeboxOutputNeeded(VP_EXECUTE_CAPS& caps);
    bool VeboxDenoiseOutputNeeded(VP_EXECUTE_CAPS& caps);
    bool VeboxHdr3DlutNeeded(VP_EXECUTE_CAPS &caps);
    bool Vebox1DlutNeeded(VP_EXECUTE_CAPS &caps);
    // In some case, STMM should not be destroyed but not be used by current workload to maintain data,
    // e.g. DI second field case.
    // If queryAssignment == true, query whether STMM needed by current workload.
    // If queryAssignment == false, query whether STMM needed to be allocated.
    bool VeboxSTMMNeeded(VP_EXECUTE_CAPS& caps, bool queryAssignment);
    virtual uint32_t GetHistogramSurfaceSize(VP_EXECUTE_CAPS& caps, uint32_t inputWidth, uint32_t inputHeight);
    virtual uint32_t Get3DLutSize(bool is33LutSizeEnabled, uint32_t &lutWidth, uint32_t &lutHeight);
    virtual uint32_t Get1DLutSize();
    virtual Mos_MemPool GetHistStatMemType(VP_EXECUTE_CAPS &caps);
    MOS_STATUS ReAllocateVeboxOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, VP_SURFACE *outputSurface, bool &allocated);
    MOS_STATUS ReAllocateVeboxDenoiseOutputSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, bool &allocated);
    MOS_STATUS ReAllocateVeboxSTMMSurface(VP_EXECUTE_CAPS& caps, VP_SURFACE *inputSurface, bool &allocated);
    void DestoryVeboxOutputSurface();
    void DestoryVeboxDenoiseOutputSurface();
    void DestoryVeboxSTMMSurface();
    virtual MOS_STATUS AssignRenderResource(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface, std::vector<VP_SURFACE *> &pastSurfaces, std::vector<VP_SURFACE *> &futureSurfaces, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe &executedFilters);
    virtual MOS_STATUS Assign3DLutKernelResource(VP_EXECUTE_CAPS &caps, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting);
    virtual MOS_STATUS AssignHVSKernelResource(VP_EXECUTE_CAPS &caps, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting);
    virtual MOS_STATUS AssignFcResources(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface,
        std::vector<VP_SURFACE *> &pastSurfaces, std::vector<VP_SURFACE *> &futureSurfaces,
        RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting);
    virtual MOS_STATUS AssignVeboxResourceForRender(VP_EXECUTE_CAPS &caps, VP_SURFACE *inputSurface, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting);
    virtual MOS_STATUS AssignVeboxResource(VP_EXECUTE_CAPS& caps, VP_SURFACE* inputSurface, VP_SURFACE* outputSurface, VP_SURFACE* pastSurface, VP_SURFACE* futureSurface,
        RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING& surfSetting, SwFilterPipe& executedFilters);
    MOS_STATUS ReAllocateVeboxStatisticsSurface(VP_SURFACE *&statisticsSurface, VP_EXECUTE_CAPS &caps, VP_SURFACE *inputSurface, uint32_t dwWidth, uint32_t dwHeight);
    void InitSurfaceConfigMap();
    void AddSurfaceConfig(bool _b64DI, bool _sfcEnable, bool _sameSample, bool _outOfBound, bool _pastRefAvailable, bool _futureRefAvailable, bool _firstDiField,
        VEBOX_SURFACE_ID _currentInputSurface, VEBOX_SURFACE_ID _pastInputSurface, VEBOX_SURFACE_ID _currentOutputSurface, VEBOX_SURFACE_ID _pastOutputSurface)
    {
        m_veboxSurfaceConfigMap.insert(std::make_pair(VEBOX_SURFACES_CONFIG(_b64DI, _sfcEnable, _sameSample, _outOfBound, _pastRefAvailable, _futureRefAvailable, _firstDiField).value, VEBOX_SURFACES(_currentInputSurface, _pastInputSurface, _currentOutputSurface, _pastOutputSurface)));
    }

    virtual MOS_STATUS GetIntermediaColorAndFormat3DLutOutput(VPHAL_CSPACE &colorSpace, MOS_FORMAT &format, SwFilterPipe &executedFilters);
    virtual MOS_STATUS GetIntermediaColorAndFormatBT2020toRGB(VP_EXECUTE_CAPS &caps, VPHAL_CSPACE &colorSpace, MOS_FORMAT &format, SwFilterPipe &executedFilters);
    virtual MOS_STATUS GetIntermediaOutputSurfaceParams(VP_EXECUTE_CAPS& caps, VP_SURFACE_PARAMS &params, SwFilterPipe &executedFilters);
    MOS_STATUS         GetIntermediaOutputSurfaceColorAndFormat(VP_EXECUTE_CAPS &caps, SwFilterPipe &executedFilters, MOS_FORMAT &format, VPHAL_CSPACE &colorSpace);
    MOS_STATUS AssignIntermediaSurface(VP_EXECUTE_CAPS& caps, SwFilterPipe &executedFilters);

    bool IsDeferredResourceDestroyNeeded()
    {
        // For 0 == m_currentPipeIndex case, the surface being destroyed should not
        // be used in current DDI call any more. So no need deferred destroyed.
        return m_currentPipeIndex > 0;
    }

    void CleanTempSurfaces();
    VP_SURFACE* GetCopyInstOfExtSurface(VP_SURFACE* surf);

    MOS_STATUS GetFormatForFcIntermediaSurface(MOS_FORMAT& format, MEDIA_CSPACE &colorSpace, SwFilterPipe &featurePipe);
    MOS_STATUS GetFcIntermediateSurfaceForOutput(VP_SURFACE *&intermediaSurface, SwFilterPipe &executedFilters);

    MOS_STATUS Allocate3DLut(VP_EXECUTE_CAPS& caps);
    MOS_STATUS AllocateResourceFor3DLutKernel(VP_EXECUTE_CAPS& caps);
    MOS_STATUS AllocateResourceForHVSKernel(VP_EXECUTE_CAPS &caps);
    MOS_STATUS AssignHdrResource(VP_EXECUTE_CAPS &caps, std::vector<VP_SURFACE *> &inputSurfaces, VP_SURFACE *outputSurface, RESOURCE_ASSIGNMENT_HINT resHint, VP_SURFACE_SETTING &surfSetting, SwFilterPipe &executedFilters);

protected:
    MOS_INTERFACE                &m_osInterface;
    VpAllocator                  &m_allocator;
    VphalFeatureReport           &m_reporting;
    vp::VpPlatformInterface      &m_vpPlatformInterface;
    vp::VpUserFeatureControl     *m_vpUserFeatureControl = nullptr;

    // Vebox Resource
    VP_SURFACE* m_veboxDenoiseOutput[VP_NUM_DN_SURFACES]     = {};            //!< Vebox Denoise output surface
    VP_SURFACE* m_veboxOutput[VP_MAX_NUM_VEBOX_SURFACES]     = {};            //!< Vebox output surface, can be reuse for DI usages
    VP_SURFACE* m_veboxSTMMSurface[VP_NUM_STMM_SURFACES]     = {};            //!< Vebox STMM input/output surface
    VP_SURFACE *m_veboxStatisticsSurface                     = nullptr;       //!< Statistics Surface for VEBOX
    VP_SURFACE *m_veboxStatisticsSurfacefor1stPassofSfc2Pass = nullptr;       //!< Statistics Surface for VEBOX for 1stPassofSfc2Pass submission
    uint32_t    m_dwVeboxPerBlockStatisticsWidth             = 0;
    uint32_t    m_dwVeboxPerBlockStatisticsHeight            = 0;
    VP_SURFACE *m_veboxRgbHistogram                          = nullptr;       //!< RGB Histogram surface for Vebox
    VP_SURFACE *m_veboxDNTempSurface                         = nullptr;       //!< Vebox DN Update kernels temp surface
    VP_SURFACE *m_veboxDNSpatialConfigSurface                = nullptr;       //!< Spatial Attributes Configuration Surface for DN kernel
    VP_SURFACE *m_vebox3DLookUpTables                        = nullptr;
    VP_SURFACE *m_vebox3DLookUpTables2D                      = nullptr;
    VP_SURFACE *m_vebox1DLookUpTables                        = nullptr;
    VP_SURFACE *m_innerTileConvertInput                      = nullptr;
    VP_SURFACE *m_veboxDnHVSTables                           = nullptr;
    VP_SURFACE *m_3DLutKernelCoefSurface                     = nullptr;       //!< Coef surface for 3DLut kernel.
    uint32_t    m_currentDnOutput                            = 0;
    uint32_t    m_currentStmmIndex                           = 0;
    uint32_t    m_veboxOutputCount                           = 2;             //!< PE on: 4 used. PE off: 2 used
    bool        m_pastDnOutputValid                          = false;         //!< true if vebox DN output of previous frame valid.
    VP_FRAME_IDS m_currentFrameIds                           = {};
    VP_FRAME_IDS m_pastFrameIds                              = {};
    bool         m_firstFrame                                = true;
    bool         m_sameSamples                               = false;
    bool         m_outOfBound                                = false;
    RECT         m_maxSrcRect                                = {};
    VEBOX_SURFACE_CONFIG_MAP m_veboxSurfaceConfigMap;
    bool        m_isHistogramReallocated                     = false;
    bool        m_isCurrentHistogramInuse                    = false;
    bool        m_isPastHistogramValid                       = false;
    uint32_t    m_imageWidthOfPastHistogram                  = 0;
    uint32_t    m_imageHeightOfPastHistogram                 = 0;
    uint32_t    m_imageWidthOfCurrentHistogram               = 0;
    uint32_t    m_imageHeightOfCurrentHistogram              = 0;
    bool        m_isFcIntermediateSurfacePrepared            = false;
    bool        m_isPastFrameVeboxDiUsed                     = false;
    VP_SURFACE *m_fcIntermediateSurface[VP_NUM_FC_INTERMEDIA_SURFACES] = {}; // Ping-pong surface for multi-layer composition.
    std::vector<VP_SURFACE *> m_intermediaSurfaces;
    std::map<uint64_t, VP_SURFACE *> m_tempSurface; // allocation handle and surface pointer pair.
    std::map<VP_SURFACE *, void *>   m_internalResourceDumpList;  // surface pointer and system memory pointer.
    // Pipe index for one DDI call.
    uint32_t    m_currentPipeIndex                           = 0;

    VP_SURFACE *m_veboxInputForLace                           = nullptr;       //!< Vebox input surface for Lace kernel to use
    VP_SURFACE *m_veboxAggregatedHistogramSurface             = nullptr;       //!< VEBOX 1D LUT surface for Vebox Gen12
    VP_SURFACE *m_veboxFrameHistogramSurface                  = nullptr;       //!< VEBOX 1D LUT surface for Vebox Gen12
    VP_SURFACE *m_veboxStdStatisticsSurface                   = nullptr;       //!< VEBOX 1D LUT surface for Vebox Gen12
    VP_SURFACE *m_veboxPwlfSurface[VP_NUM_LACE_PWLF_SURFACES] = {};            //!< VEBOX 1D LUT surface for Vebox Gen12
    VP_SURFACE *m_veboxWeitCoefSurface                        = nullptr;       //!< VEBOX 1D LUT surface for Vebox Gen12
    VP_SURFACE *m_veboxGlobalToneMappingCurveLUTSurface       = nullptr;       //!< VEBOX 1D LUT surface for Vebox Gen12
    VP_SURFACE *m_temperalInput                               = nullptr;

    // Fc Resource
    VP_SURFACE *m_cmfcCoeff                                                    = nullptr;
    VP_SURFACE *m_fcIntermediaSurfaceOutput                                    = nullptr;
    VP_SURFACE *m_fcIntermediaSurfaceInput[VP_COMP_MAX_LAYERS]                 = {};
    VP_SURFACE *m_fcSeparateIntermediaSurfaceSecPlaneInput[VP_COMP_MAX_LAYERS] = {};
    //for decompreesion sync on interlace input of FC
    VP_SURFACE *m_decompressionSyncSurface                                  = nullptr;
    // Hdr Resource
    VphdrResourceManager *m_hdrResourceManager                = nullptr;
    MediaUserSettingSharedPtr m_userSettingPtr                = nullptr;   //!< usersettingInstance

    MediaCopyWrapper *m_mediaCopyWrapper                      = nullptr;

    MEDIA_CLASS_DEFINE_END(vp__VpResourceManager)
};
}
#endif // _VP_RESOURCE_MANAGER_H__
