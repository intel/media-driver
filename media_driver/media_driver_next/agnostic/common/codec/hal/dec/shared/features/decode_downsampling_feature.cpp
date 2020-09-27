/*
* Copyright (c) 2020, Intel Corporation
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
//! \file     decode_downsampling_feature.cpp
//! \brief    Defines the common interface for decode downsampling features
//! \details  The decode downsampling feature interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#include "decode_downsampling_feature.h"
#include "decode_utils.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
DecodeDownSamplingFeature::DecodeDownSamplingFeature(
    MediaFeatureManager *featureManager, DecodeAllocator *allocator, CodechalHwInterface *hwInterface):
    m_hwInterface(hwInterface), m_allocator(allocator)
{
    m_featureManager = featureManager;
}

DecodeDownSamplingFeature::~DecodeDownSamplingFeature()
{
}

MOS_STATUS DecodeDownSamplingFeature::Init(void *setting)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(m_featureManager);
    DECODE_CHK_NULL(m_allocator);

    DECODE_CHK_STATUS(m_internalTargets.Init(*m_allocator));

    m_basicFeature = dynamic_cast<DecodeBasicFeature*>(m_featureManager->GetFeature(FeatureIDs::basicFeature));
    DECODE_CHK_NULL(m_basicFeature);

    MOS_ZeroMemory(&m_outputSurface, sizeof(m_outputSurface));
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeDownSamplingFeature::Update(void *params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;

    if (decodeParams->m_refFrameCnt == 0)
    {
        m_inputSurface  = nullptr;
        return MOS_STATUS_SUCCESS;
    }

    DECODE_CHK_NULL(decodeParams->m_procParams);
    DecodeProcessingParams *procParams = (DecodeProcessingParams *)decodeParams->m_procParams;

    m_chromaSitingType             = procParams->m_chromaSitingType;
    m_rotationState                = procParams->m_rotationState;
    m_blendState                   = procParams->m_blendState;
    m_mirrorState                  = procParams->m_mirrorState;
    m_isReferenceOnlyPattern       = procParams->m_isReferenceOnlyPattern;

    DECODE_CHK_NULL(procParams->m_outputSurface);
    m_outputSurface = *(procParams->m_outputSurface);
    DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(&m_outputSurface));

    m_outputSurfaceRegion.m_x      = procParams->m_outputSurfaceRegion.m_x;
    m_outputSurfaceRegion.m_y      = procParams->m_outputSurfaceRegion.m_y;
    m_outputSurfaceRegion.m_width  = procParams->m_outputSurfaceRegion.m_width;
    m_outputSurfaceRegion.m_height = procParams->m_outputSurfaceRegion.m_height;

    if (procParams->m_isSourceSurfAllocated)
    {
        m_outputSurfaceRegion.m_width  = m_outputSurface.dwWidth;
        m_outputSurfaceRegion.m_height = m_outputSurface.dwHeight;

        DECODE_CHK_NULL(procParams->m_inputSurface);
        m_inputSurface = procParams->m_inputSurface;
        DECODE_CHK_STATUS(m_allocator->GetSurfaceInfo(m_inputSurface));

        m_inputSurfaceRegion.m_x      = 0;
        m_inputSurfaceRegion.m_y      = 0;
        m_inputSurfaceRegion.m_width  = m_inputSurface->dwWidth;
        m_inputSurfaceRegion.m_height = m_inputSurface->dwHeight;
    }
    else
    {
        if (m_basicFeature->m_curRenderPic.FrameIdx >= decodeParams->m_refFrameCnt)
        {
            DECODE_ASSERTMESSAGE("Invalid Downsampling Reference Frame Index !");
            return MOS_STATUS_INVALID_PARAMETER;
        }

        DECODE_CHK_STATUS(UpdateInternalTargets(*m_basicFeature));

        m_inputSurface = m_internalTargets.GetCurSurf();
        DECODE_CHK_NULL(m_inputSurface);

        m_inputSurfaceRegion.m_x      = 0;
        m_inputSurfaceRegion.m_y      = 0;
        m_inputSurfaceRegion.m_width  = m_basicFeature->m_width;
        m_inputSurfaceRegion.m_height = m_basicFeature->m_height;
    }

    // Update decode output in basic feature
    DECODE_CHK_STATUS(UpdateDecodeTarget(*m_inputSurface));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS DecodeDownSamplingFeature::UpdateInternalTargets(DecodeBasicFeature &basicFeature)
{
    uint32_t curFrameIdx = basicFeature.m_curRenderPic.FrameIdx;

    std::vector<uint32_t> refFrameList;
    DECODE_CHK_STATUS(GetRefFrameList(refFrameList));
    DECODE_CHK_STATUS(m_internalTargets.UpdateRefList(curFrameIdx, refFrameList));

    MOS_SURFACE surface;
    MOS_ZeroMemory(&surface, sizeof(surface));
    surface.dwWidth  = basicFeature.m_width;
    surface.dwHeight = basicFeature.m_height;
    DECODE_CHK_STATUS(GetDecodeTargetFormat(surface.Format));
    DECODE_CHK_STATUS(m_internalTargets.ActiveCurSurf(curFrameIdx, &surface, resourceOutputPicture));

    return MOS_STATUS_SUCCESS;
}

}

#endif
