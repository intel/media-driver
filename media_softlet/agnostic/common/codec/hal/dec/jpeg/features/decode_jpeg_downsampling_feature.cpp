/*
* Copyright (c) 2021, Intel Corporation
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
//! \file     decode_jpeg_downsampling_feature.cpp
//! \brief    Defines the interface for Jpeg decode downsampling feature
//! \details  The Jpeg decode downsampling feature interface is maintaining the down sampling context.
//!
#include "decode_jpeg_downsampling_feature.h"
#include "decode_jpeg_basic_feature.h"
#include "decode_utils.h"

#ifdef _DECODE_PROCESSING_SUPPORTED

namespace decode
{
JpegDownSamplingFeature::JpegDownSamplingFeature(MediaFeatureManager *featureManager, DecodeAllocator *allocator,
    PMOS_INTERFACE osInterface) : DecodeDownSamplingFeature(featureManager, allocator, osInterface)
{
    MOS_ZeroMemory(&m_sfcInSurface, sizeof(m_sfcInSurface));
}

JpegDownSamplingFeature::~JpegDownSamplingFeature()
{
}

MOS_STATUS JpegDownSamplingFeature::GetDecodeTargetSize(SurfaceWidthT &width, SurfaceHeightT &height)
{
    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(jpegBasicFeature);

    width  = jpegBasicFeature->m_destSurface.dwWidth;
    height = jpegBasicFeature->m_destSurface.dwHeight;
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDownSamplingFeature::UpdateDecodeTarget(MOS_SURFACE& surface)
{
    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(jpegBasicFeature);
    DECODE_CHK_STATUS(GetDecodeTargetSize(surface.dwWidth, surface.dwHeight));
    DECODE_CHK_STATUS(GetInputSurfFormat(m_inputSurface));
    DECODE_CHK_STATUS(GetDecodeTargetFormat(m_outputSurface.Format));

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDownSamplingFeature::Update(void* params)
{
    DECODE_FUNC_CALL();
    DECODE_CHK_NULL(params);

    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(jpegBasicFeature);

    if (MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrSFCPipe) &&
        !MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrDisableVDBox2SFC) &&
        m_basicFeature->m_destSurface.Format == Format_A8R8G8B8 &&  // Currently only support this SFC usage in JPEG
        (jpegBasicFeature->m_jpegPicParams->m_interleavedData ||    // SFC only support interleaved single scan (YUV400 is excluded for "interleaved" limitation)
        jpegBasicFeature->m_jpegPicParams->m_chromaType == jpegYUV400) &&
        jpegBasicFeature->m_jpegPicParams->m_totalScans == 1)
    {
        CodechalDecodeParams *decodeParams = (CodechalDecodeParams *)params;
        if (decodeParams->m_procParams == nullptr) //procParams not givin by app
        {
            DecodeProcessingParams procParams;
            MOS_ZeroMemory(&procParams, sizeof(DecodeProcessingParams));
            m_sfcInSurface.dwPitch    = MOS_ALIGN_CEIL(jpegBasicFeature->m_destSurface.dwWidth, CODECHAL_SURFACE_PITCH_ALIGNMENT);
            procParams.m_inputSurface = &m_sfcInSurface;
            procParams.m_inputSurfaceRegion.m_width   = jpegBasicFeature->m_destSurface.dwWidth;
            procParams.m_inputSurfaceRegion.m_height   = jpegBasicFeature->m_destSurface.dwHeight;
            procParams.m_inputSurface->OsResource     = jpegBasicFeature->m_destSurface.OsResource;
            procParams.m_outputSurface = &jpegBasicFeature->m_destSurface;
            procParams.m_outputSurfaceRegion.m_width  = jpegBasicFeature->m_destSurface.dwWidth;
            procParams.m_outputSurfaceRegion.m_height = jpegBasicFeature->m_destSurface.dwHeight;

            decodeParams->m_procParams = &procParams;

            DecodeDownSamplingFeature::Update(decodeParams);
            decodeParams->m_procParams = nullptr;    //to in case duplicate free.
        }
        else
        {
            DecodeDownSamplingFeature::Update(decodeParams);
        }
    }
    else
    {
        if ((!MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrSFCPipe) ||
            MEDIA_IS_SKU(m_osInterface->pfnGetSkuTable(m_osInterface), FtrDisableVDBox2SFC)) &&
            m_basicFeature->m_destSurface.Format == Format_A8R8G8B8)
        {
            DECODE_ASSERTMESSAGE("Don't support using SFC to convert.");
            return MOS_STATUS_INVALID_PARAMETER;
        }
    }

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDownSamplingFeature::GetRefFrameList(std::vector<uint32_t> &refFrameList)
{
    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDownSamplingFeature::GetDecodeTargetFormat(MOS_FORMAT &format)
{
    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(jpegBasicFeature);
    
    jpegBasicFeature->GetRenderTargetFormat(&format);

    return MOS_STATUS_SUCCESS;
}

MOS_STATUS JpegDownSamplingFeature::GetInputSurfFormat(PMOS_SURFACE surface)
{
    JpegBasicFeature *jpegBasicFeature = dynamic_cast<JpegBasicFeature *>(m_basicFeature);
    DECODE_CHK_NULL(jpegBasicFeature);
    switch (jpegBasicFeature->m_jpegPicParams->m_chromaType)
    {
    case jpegYUV400:
        surface->Format = Format_400P;
        break;
    case jpegYUV411:
        m_sfcInSurface.Format = Format_411P;
        break;
    case jpegYUV420:
        surface->Format = Format_NV12;
        surface->VPlaneOffset.iYOffset =
            MOS_ALIGN_CEIL(jpegBasicFeature->m_destSurface.dwHeight, MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY) + (jpegBasicFeature->m_destSurface.dwHeight >> 1);
        break;
    case jpegYUV422H2Y:
    case jpegYUV422H4Y:
        surface->Format = Format_422H;
        surface->VPlaneOffset.iYOffset =
            MOS_ALIGN_CEIL(jpegBasicFeature->m_destSurface.dwHeight, MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY) + (jpegBasicFeature->m_destSurface.dwHeight >> 1);
        break;
    case jpegYUV444:
    case jpegRGB:
    case jpegBGR:
        surface->Format = Format_444P;
        surface->VPlaneOffset.iYOffset =
            MOS_ALIGN_CEIL(jpegBasicFeature->m_destSurface.dwHeight, MHW_VDBOX_MFX_UV_PLANE_ALIGNMENT_LEGACY) + jpegBasicFeature->m_destSurface.dwHeight;
        break;
    default:
        surface->Format = Format_Invalid;
        return MOS_STATUS_INVALID_PARAMETER; //Format not support, exit downsampling feature.
    }
    return MOS_STATUS_SUCCESS;
}

}

#endif
