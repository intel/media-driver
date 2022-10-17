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
//! \file     decode_downsampling_feature.h
//! \brief    Defines the common interface for decode downsampling feature
//! \details  The decode downsampling feature interface is further sub-divided by codec standard,
//!           this file is for the base interface which is shared by all codecs.
//!
#ifndef __DECODE_DOWNSAMPLING_FEATURE_H__
#define __DECODE_DOWNSAMPLING_FEATURE_H__

#include "codec_def_decode.h"
#include "decode_allocator.h"
#include "media_feature.h"
#include "codec_hw_next.h"
#include "codechal_setting.h"
#include "decode_internal_target.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
class DecodeDownSamplingFeature: public MediaFeature
{
public:
    using SurfaceWidthT  = decltype(MOS_SURFACE::dwWidth);
    using SurfaceHeightT = decltype(MOS_SURFACE::dwHeight);

    DecodeDownSamplingFeature(MediaFeatureManager *featureManager, DecodeAllocator *allocator, PMOS_INTERFACE osInterface);
    virtual ~DecodeDownSamplingFeature();

    //!
    //! \brief  Init decode basic parameter
    //! \param  [in] setting
    //!         Pointer to CodechalSetting
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Init(void *setting);

    //!
    //! \brief  Update decode basic feature
    //! \param  [in] params
    //!         Pointer to DecoderParams
    //! \return MOS_STATUS
    //!         MOS_STATUS_SUCCESS if success, else fail reason
    //!
    virtual MOS_STATUS Update(void *params);

    virtual MOS_STATUS DumpSfcOutputs(CodechalDebugInterface* debugInterface);

    // Downsampling input
    PMOS_SURFACE   m_inputSurface = nullptr;
    CodecRectangle m_inputSurfaceRegion = {};
    uint32_t       m_chromaSitingType = 0;

    // Downsampling output
    MOS_SURFACE    m_outputSurface;
    CodecRectangle m_outputSurfaceRegion = {};

    // Downsampling parameters
    uint32_t       m_rotationState = 0;
    uint32_t       m_blendState = 0;
    uint32_t       m_mirrorState = 0;
    bool           m_isInputSurfAllocated = false;
    bool           m_isReferenceOnlyPattern = false;

    CODECHAL_SCALING_MODE  m_scalingMode = CODECHAL_SCALING_NEAREST;

    // Histogram
    PMOS_BUFFER    m_histogramBuffer   = nullptr;  // SFC histogram internal buffer for current frame
    PMOS_SURFACE   m_histogramDestSurf = nullptr;  // SFC histogram dest surface
    bool           m_histogramDebug    = false;
    const uint32_t m_histogramBinWidth = 4;

#if (_DEBUG || _RELEASE_INTERNAL)
    MOS_SURFACE    m_outputSurfaceList[DecodeBasicFeature::m_maxFrameIndex] = {}; //! \brief Downsampled surfaces
#endif

protected:
    virtual MOS_STATUS UpdateInternalTargets(DecodeBasicFeature &basicFeature);

    virtual MOS_STATUS GetRefFrameList(std::vector<uint32_t> &refFrameList) = 0;
    virtual MOS_STATUS GetDecodeTargetSize(SurfaceWidthT &width, SurfaceHeightT &height) = 0;
    virtual MOS_STATUS GetDecodeTargetFormat(MOS_FORMAT &format) = 0;
    virtual MOS_STATUS UpdateDecodeTarget(MOS_SURFACE &surface) = 0;
    PMOS_BUFFER        AllocateHistogramBuffer(uint8_t frameIndex);

    PMOS_INTERFACE       m_osInterface  = nullptr;
    DecodeAllocator     *m_allocator    = nullptr;
    DecodeBasicFeature  *m_basicFeature = nullptr;

    InternalTargets      m_internalTargets; //!< Internal targets for downsampling input if application dosen't prepare
    PMOS_BUFFER          m_histogramBufferList[DecodeBasicFeature::m_maxFrameIndex] = {};  //! \brief Internal histogram output buffer list

MEDIA_CLASS_DEFINE_END(decode__DecodeDownSamplingFeature)
};
}//decode

#endif // !_DECODE_PROCESSING_SUPPORTED

#endif // !__DECODE_DOWNSAMPLING_FEATURE_H__
