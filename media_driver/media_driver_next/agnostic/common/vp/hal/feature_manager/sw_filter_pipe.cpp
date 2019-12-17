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
//! \file     sw_filter_pipe.cpp
//! \brief    Defines the common interface for vp features manager
//! \details  The vp manager is further sub-divided by vp type
//!           this file is for the base interface which is shared by all components.
//!
#include "sw_filter_pipe.h"
using namespace vp;

/****************************************************************************************************/
/*                                      SwFilterPipe                                                */
/****************************************************************************************************/

SwFilterPipe::SwFilterPipe()
{
}

SwFilterPipe::~SwFilterPipe()
{
}

void SwFilterPipe::InitMosSurface(MOS_SURFACE &surf, VPHAL_SURFACE &vphalSurf)
{
    surf.TileType = vphalSurf.TileType;
    surf.TileModeGMM = vphalSurf.TileModeGMM;
    surf.bGMMTileEnabled = vphalSurf.bGMMTileEnabled;
}

void SwFilterPipe::InitSwFilterSurfaceInfo(SwFilterSurfaceInfo &surfInfo, VPHAL_SURFACE &vphalSurf)
{
    MOS_SURFACE &surf = surfInfo.m_OsSurface;
    InitMosSurface(surf, vphalSurf);

    surfInfo.ColorSpace = vphalSurf.ColorSpace;
    surfInfo.ExtendedGamut = vphalSurf.ExtendedGamut;
    surfInfo.Palette = vphalSurf.Palette;
    surfInfo.bQueryVariance = vphalSurf.bQueryVariance;
    surfInfo.FrameID = vphalSurf.FrameID;
    surfInfo.uFwdRefCount = vphalSurf.uFwdRefCount;  
    surfInfo.uBwdRefCount = vphalSurf.uBwdRefCount;
    surfInfo.pFwdRef = vphalSurf.pFwdRef;
    surfInfo.pBwdRef = vphalSurf.pBwdRef;
    surfInfo.iLayerID = vphalSurf.iLayerID;
    surfInfo.SurfType = vphalSurf.SurfType;
    surfInfo.SampleType = vphalSurf.SampleType;
    surfInfo.ChromaSiting = vphalSurf.ChromaSiting;
}

MOS_STATUS SwFilterPipe::Initialize(VP_PIPELINE_PARAMS &params, FeatureRule &featureRule)
{
    uint32_t i = 0;
    for (i = 0; i < params.uSrcCount; ++i)
    {
        m_InputSurfaces.push_back(SwFilterSurfaceInfo());
        SwFilterSurfaceInfo &surfInfo = m_InputSurfaces[m_InputSurfaces.size() - 1];
        VPHAL_SURFACE &vphalSurf = *params.pSrc[i];

        InitSwFilterSurfaceInfo(surfInfo, vphalSurf);

        // Initialize m_InputPipes and m_OutputPipes.
    }

    for (i = 0; i < params.uDstCount; ++i)
    {
        m_OutputSurfaces.push_back(SwFilterSurfaceInfo());
        SwFilterSurfaceInfo &surfInfo = m_OutputSurfaces[m_OutputSurfaces.size() - 1];
        VPHAL_SURFACE &vphalSurf = *params.pTarget[i];

        InitSwFilterSurfaceInfo(surfInfo, vphalSurf);
    }

    return MOS_STATUS_SUCCESS;
}

bool SwFilterPipe::IsEmpty()
{
    return true;
}
