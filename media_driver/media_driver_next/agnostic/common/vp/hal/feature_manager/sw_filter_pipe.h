/*
* Copyright (c) 2019 - 2020, Intel Corporation
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
//! \file     vp_feature_manager.h
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#ifndef __SW_FILTER_PIPE_H__
#define __SW_FILTER_PIPE_H__

#include "media_feature_manager.h"
#include "vp_utils.h"
#include "vp_pipeline_common.h"
#include "vp_allocator.h"

#include <vector>
#include "sw_filter.h"

namespace vp
{
// Filters without order.

#define MAX_LAYER_COUNT VPHAL_MAX_SOURCES

class VpInterface;

struct FeatureSet
{
    std::vector<FeatureType> m_Features;
};

struct FeatureSubRule
{
    std::vector<FeatureSet> m_Rule;
};

struct FeatureRule
{
    std::vector<FeatureSubRule> m_InputPipes;
    std::vector<FeatureSubRule> m_OutputPipes;
};

class SwFilterSubPipe
{
public:
    SwFilterSubPipe();
    virtual ~SwFilterSubPipe();
    MOS_STATUS Clean();
    MOS_STATUS Update(VP_SURFACE *inputSurf, VP_SURFACE *outputSurf);
    SwFilter *GetSwFilter(FeatureType type);
    MOS_STATUS AddSwFilterOrdered(SwFilter *swFilter, bool useNewSwFilterSet);
    MOS_STATUS AddSwFilterUnordered(SwFilter *swFilter);
    bool IsEmpty()
    {
        bool ret = false;

        if (m_OrderedFilters.size() == 0 &&
            m_UnorderedFilters.IsEmpty())
        {
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    std::vector<SwFilterSet *> m_OrderedFilters;    // For features in featureRule
    SwFilterSet m_UnorderedFilters;                 // For features not in featureRule
};

enum SwFilterPipeType
{
    SwFilterPipeTypeInvalid = 0,
    SwFilterPipeType1To1,
    SwFilterPipeTypeNTo1,
    SwFilterPipeType1ToN,
    SwFilterPipeType0To1,
    NumOfSwFilterPipeType
};

class SwFilterFeatureHandler
{
public:
    SwFilterFeatureHandler(VpInterface &vpInterface, FeatureType type);
    virtual ~SwFilterFeatureHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
    virtual MOS_STATUS CreateSwFilter(SwFilter *&swFilter, VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
protected:
    VpInterface     &m_vpInterface;
    FeatureType     m_type;
};

class SwFilterCscHandler : public SwFilterFeatureHandler
{
public:
    SwFilterCscHandler(VpInterface &vpInterface);
    virtual ~SwFilterCscHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
};

class SwFilterRotMirHandler : public SwFilterFeatureHandler
{
public:
    SwFilterRotMirHandler(VpInterface &vpInterface);
    virtual ~SwFilterRotMirHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
};

class SwFilterScalingHandler : public SwFilterFeatureHandler
{
public:
    SwFilterScalingHandler(VpInterface &vpInterface);
    virtual ~SwFilterScalingHandler();
    virtual bool IsFeatureEnabled(VP_PIPELINE_PARAMS &params, bool isInputPipe, int surfIndex, SwFilterPipeType pipeType);
};

class SwFilterPipe
{
public:
    SwFilterPipe(VpInterface &vpInterface);
    virtual ~SwFilterPipe();
    MOS_STATUS Initialize(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule);
    void UpdateSwFilterPipeType();
    MOS_STATUS Clean();

    bool IsEmpty();
    bool IsPrimaryEmpty();

    MOS_STATUS RegisterFeatures();
    MOS_STATUS UnregisterFeatures();

    MOS_STATUS ConfigFeaturesToPipe(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule, bool isInputPipe);
    MOS_STATUS ConfigFeatures(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule);
    MOS_STATUS UpdateFeatures(bool isInputPipe, uint32_t pipeIndex);

    SwFilterPipeType GetSwFilterPipeType()
    {
        return m_swFilterPipeType;
    }

    SwFilter *GetSwFilter(bool isInputPipe, int index, FeatureType type);
    SwFilterSubPipe *GetSwFilterSubPipe(bool isInputPipe, int index);
    SwFilterSubPipe *GetSwFilterPrimaryPipe(uint32_t& index);

    // useNewSwFilterSet: true if insert new swFilterSet in pipe, otherwise, reuse the last one in pipe.
    MOS_STATUS AddSwFilterOrdered(SwFilter *swFilter, bool isInputPipe, int index, bool useNewSwFilterSet);
    MOS_STATUS AddSwFilterUnordered(SwFilter *swFilter, bool isInputPipe, int index);
    MOS_STATUS RemoveSwFilter(SwFilter *swFilter);
    VP_SURFACE *GetSurface(bool isInputSurface, uint32_t index);
    VP_SURFACE *RemoveSurface(bool isInputSurface, uint32_t index);
    MOS_STATUS AddSurface(VP_SURFACE *&surf, bool isInputSurface, uint32_t index);
    MOS_STATUS Update();
    uint32_t GetSurfaceCount(bool isInputSurface);

protected:
    MOS_STATUS CleanFeaturesFromPipe(bool isInputPipe, uint32_t index);
    MOS_STATUS CleanFeaturesFromPipe(bool isInputPipe);
    MOS_STATUS CleanFeatures();
    MOS_STATUS RemoveUnusedLayers(bool bUpdateInput);

    std::vector<SwFilterSubPipe *>      m_InputPipes;       // For features on input surfaces.
    std::vector<SwFilterSubPipe *>      m_OutputPipes;      // For features on output surfaces.

    std::vector<VP_SURFACE *>           m_InputSurfaces;
    std::vector<VP_SURFACE *>           m_OutputSurfaces;
    std::vector<VP_SURFACE *>           m_PreviousSurface;
    std::vector<VP_SURFACE *>           m_NextSurface;

    VpInterface                         &m_vpInterface;
    std::map<FeatureType, SwFilterFeatureHandler*> m_featureHandler;

    bool                                m_isFeatureRegistered = false;
    SwFilterPipeType                    m_swFilterPipeType = SwFilterPipeTypeInvalid;
};


}
#endif // !__SW_FILTER_PIPE_H__
