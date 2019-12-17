/*
* Copyright (c) 2019, Intel Corporation
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

struct SwFilterSet
{
    std::vector<SwFilter *> m_SwFilters;
};

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

struct SwFilterSubPipe
{
    std::vector<SwFilterSet *> m_OrderedFilters;    // For features in featureRule
    SwFilterSet m_DisorderedFilters;                // For features not in featureRule
};

enum SwFilterPipeType
{
    SwFilterPipeTypeInvalid = 0,
    SwFilterPipeType1To1,
    SwFilterPipeTypeNTo1,
    SwFilterPipeType1ToN,
    NumOfSwFilterPipeType
};

struct SwFilterSurfaceInfo
{
    MOS_SURFACE m_OsSurface;

    // Color Information
    VPHAL_CSPACE                ColorSpace;         //!<Color Space
    bool                        ExtendedGamut;      //!<Extended Gamut Flag
    VPHAL_PALETTE               Palette;            //!<Palette data

    // Rendering parameters

    bool                        bQueryVariance;     //!< enable variance query  For DI???

    // Frame ID and reference samples -> for advanced processing
    int32_t                     FrameID;
    uint32_t                    uFwdRefCount;
    uint32_t                    uBwdRefCount;
    PVPHAL_SURFACE              pFwdRef;
    PVPHAL_SURFACE              pBwdRef;

    //// VPHAL_SURFACE Linked list
    //PVPHAL_SURFACE              pNext;

    //--------------------------------------
    // FIELDS TO BE SETUP BY VPHAL int32_tERNALLY
    //--------------------------------------
    int32_t                     iLayerID;           //!<  Layer index (0-based index)

    //--------------------------------------
    // FIELDS TO BE PROVIDED BY DDI
    //--------------------------------------
    // Sample information
    VPHAL_SURFACE_TYPE          SurfType;           //!<  Surface type (context)
    VPHAL_SAMPLE_TYPE           SampleType;         //!<  Interlaced/Progressive sample type

    // Chroma siting
    uint32_t                    ChromaSiting;
};

class SwFilterPipe
{
public:
    SwFilterPipe();
    virtual ~SwFilterPipe();
    MOS_STATUS Initialize(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule);
    bool IsEmpty();

private:
    void InitMosSurface(MOS_SURFACE &surf, VPHAL_SURFACE &vphalSurf);
    void InitSwFilterSurfaceInfo(SwFilterSurfaceInfo &surfInfo, VPHAL_SURFACE &vphalSurf);

    std::vector<SwFilterSubPipe *> m_InputPipes;        // For features on input surfaces.
    std::vector<SwFilterSubPipe *> m_OutputPipes;       // For features on output surfaces.

    std::vector<SwFilterSurfaceInfo> m_InputSurfaces;
    std::vector<SwFilterSurfaceInfo> m_OutputSurfaces;
    std::vector<SwFilterSurfaceInfo> m_PreviousSurface;
    std::vector<SwFilterSurfaceInfo> m_NextSurface;
};


}
#endif // !__SW_FILTER_PIPE_H__
